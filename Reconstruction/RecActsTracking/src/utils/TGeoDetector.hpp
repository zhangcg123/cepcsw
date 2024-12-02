#include "Acts/Geometry/CylinderVolumeBuilder.hpp"
#include "Acts/Geometry/CylinderVolumeHelper.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Geometry/ITrackingVolumeBuilder.hpp"
#include "Acts/Geometry/LayerArrayCreator.hpp"
#include "Acts/Geometry/LayerCreator.hpp"
#include "Acts/Geometry/PassiveLayerBuilder.hpp"
#include "Acts/Geometry/ProtoLayerHelper.hpp"
#include "Acts/Geometry/SurfaceArrayCreator.hpp"
#include "Acts/Geometry/SurfaceBinningMatcher.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Geometry/TrackingGeometryBuilder.hpp"
#include "Acts/Geometry/TrackingVolumeArrayCreator.hpp"

#include "Acts/Plugins/TGeo/TGeoCylinderDiscSplitter.hpp"
#include "Acts/Plugins/TGeo/TGeoLayerBuilder.hpp"
#include "Acts/Plugins/Json/JsonMaterialDecorator.hpp"
#include "Acts/Plugins/Json/ActsJson.hpp"

#include "Acts/Utilities/BinningType.hpp"
#include "Acts/Utilities/Logger.hpp"

#include <boost/program_options.hpp>
#include <nlohmann/json.hpp>

#include <cstddef>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace Acts
{
    class TGeoDetectorElement;
    class TrackingGeometry;
    class IMaterialDecorator;
}

/// ----------------------------
///        pre definitions 
/// ----------------------------

/// Half open [lower,upper) interval type for a single user option.
///
/// A missing limit represents an unbounded upper or lower limit. With just
/// one defined limit the interval is just a lower/upper bound; with both
/// limits undefined, the interval is unbounded everywhere and thus contains
/// all possible values.
struct Interval
{
    std::optional<double> lower;
    std::optional<double> upper;
};

/// Extract an interval from an input of the form 'lower:upper'.
///
/// An input of the form `lower:` or `:upper` sets just one of the limits. Any
/// other input leads to an unbounded interval.
///
/// @note The more common range notation uses `lower-upper` but the `-`
///   separator complicates the parsing of negative values.
std::istream& operator>>(std::istream& is, Interval& interval);

/// Print an interval as `lower:upper`.
std::ostream& operator<<(std::ostream& os, const Interval& interval);

struct TGeoConfig {
    Acts::Logging::Level surfaceLogLevel = Acts::Logging::WARNING;
    Acts::Logging::Level layerLogLevel   = Acts::Logging::WARNING;
    Acts::Logging::Level volumeLogLevel  = Acts::Logging::WARNING;

    std::string fileName;
    bool buildBeamPipe = false;
    double beamPipeRadius{0};
    double beamPipeHalflengthZ{0};
    double beamPipeLayerThickness{0};
    double beamPipeEnvelopeR{1.0};
    double layerEnvelopeR{1.0};

    double unitScalor = 1.0;

    Acts::TGeoLayerBuilder::ElementFactory elementFactory =
        Acts::TGeoLayerBuilder::defaultElementFactory;

    /// Optional geometry identifier hook to be used during closure
    std::shared_ptr<const Acts::GeometryIdentifierHook> geometryIdentifierHook =
    std::make_shared<Acts::GeometryIdentifierHook>();

    enum SubVolume : std::size_t { Negative = 0, Central, Positive };

    template <typename T>
    struct LayerTriplet
    {
        LayerTriplet() = default;

        LayerTriplet(T value)
            : negative{value}, central{value}, positive{value} {}

        LayerTriplet(T _negative, T _central, T _positive)
            : negative{_negative}, central{_central}, positive{_positive} {}

        T negative;
        T central;
        T positive;

        T& at(SubVolume i)
        {
            switch (i)
            {
                case Negative: return negative;
                case Central:  return central;
                case Positive: return positive;
                default: throw std::invalid_argument{"Unknown index"};
            }
        }

        const T& at(SubVolume i) const
        {
            switch (i)
            {
                case Negative: return negative;
                case Central: return central;
                case Positive: return positive;
                default: throw std::invalid_argument{"Unknown index"};
            }
        }
    };

    struct Volume {
        std::string name;
        LayerTriplet<bool> layers{false};
        LayerTriplet<std::string> subVolumeName;
        LayerTriplet<std::vector<std::string>> sensitiveNames;
        LayerTriplet<std::string> sensitiveAxes;
        LayerTriplet<Interval> rRange;
        LayerTriplet<Interval> zRange;
        LayerTriplet<double> splitTolR{0};
        LayerTriplet<double> splitTolZ{0};
        LayerTriplet<std::vector<std::pair<int, Acts::BinningType>>> binning0;
        LayerTriplet<std::vector<std::pair<int, Acts::BinningType>>> binning1;

        Interval binToleranceR;
        Interval binTolerancePhi;
        Interval binToleranceZ;

        bool cylinderDiscSplit = false;
        unsigned int cylinderNZSegments = 0;
        unsigned int cylinderNPhiSegments = 0;
        unsigned int discNRSegments = 0;
        unsigned int discNPhiSegments = 0;

        bool itkModuleSplit = false;
        std::map<std::string, unsigned int> barrelMap;
        std::map<std::string, std::vector<std::pair<double, double>>> discMap;
        /// pairs of regular expressions to match sensor names and category keys
        /// for either the barrelMap or the discMap
        /// @TODO in principle vector<pair< > > would be good enough
        std::map<std::string, std::string> splitPatterns;
    };

    std::vector<Volume> volumes;
};

/// ------------------------------------------------------------------------------------------------
///     @note nlohmann::json must define functions *from_json* && *to_json* for json conversion
/// ------------------------------------------------------------------------------------------------

namespace Acts {
/// Read & Write config for cylinder/disc module splitter
void from_json(const nlohmann::json& j,
               Acts::TGeoCylinderDiscSplitter::Config& cdc)
{
    /// Number of segments in phi for a disc
    cdc.cylinderPhiSegments = j.at("geo-tgeo-cyl-nphi-segs");
    /// Number of segments in r for a disk
    cdc.cylinderLongitudinalSegments = j.at("geo-tgeo-cyl-nz-segs");
    /// Number of segments in phi for a disc
    cdc.discPhiSegments = j.at("geo-tgeo-disc-nphi-segs");
    /// Number of segments in r for a disk
    cdc.discRadialSegments = j.at("geo-tgeo-disc-nr-segs");
}

void to_json(nlohmann::json& j,
             const Acts::TGeoCylinderDiscSplitter::Config& cdc)
{
    j = nlohmann::json{{"geo-tgeo-cyl-nphi-segs", cdc.cylinderPhiSegments},
                       {"geo-tgeo-cyl-nz-segs", cdc.cylinderLongitudinalSegments},
                       {"geo-tgeo-disc-nphi-segs", cdc.discPhiSegments},
                       {"geo-tgeo-disc-nr-segs", cdc.discRadialSegments}};
}

// enum specialization by nlohman library
NLOHMANN_JSON_SERIALIZE_ENUM(Acts::BinningType,
                            {
                                {Acts::BinningType::equidistant, "equidistant"},
                                {Acts::BinningType::arbitrary, "arbitrary"},
                            })
}

/// Read & Write config for options interval
void from_json(const nlohmann::json& j,
               Interval& interval)
{
    interval.lower = j.at("lower");
    interval.upper = j.at("upper");
}

void to_json(nlohmann::json& j,
             const Interval& interval)
{
    j = nlohmann::json{{"lower", interval.lower.value_or(0)},
                       {"upper", interval.upper.value_or(0)}};
}

/// Read & Write layer configuration triplets
template <typename T>
void from_json(const nlohmann::json& j,
               TGeoConfig::LayerTriplet<T>& ltr)
{
    ltr.negative = j.at("negative").get<T>();
    ltr.central  = j.at("central").get<T>();
    ltr.positive = j.at("positive").get<T>();
}

template <typename T>
void to_json(nlohmann::json& j,
             const TGeoConfig::LayerTriplet<T>& ltr)
{
    j = nlohmann::json{{"negative", ltr.negative},
                       {"central", ltr.central},
                       {"positive", ltr.positive}};
}

/// Read & Write volume struct
void from_json(const nlohmann::json& j, TGeoConfig::Volume& vol) {
    // subdetector selection
    vol.name = j.at("geo-tgeo-volume-name");

    // configure surface autobinning
    vol.binToleranceR = j.at("geo-tgeo-sfbin-r-tolerance");
    vol.binToleranceZ = j.at("geo-tgeo-sfbin-z-tolerance");
    vol.binTolerancePhi = j.at("geo-tgeo-sfbin-phi-tolerance");

    // Fill layer triplets
    vol.layers = j.at("geo-tgeo-volume-layers");
    vol.subVolumeName = j.at("geo-tgeo-subvolume-names");
    vol.sensitiveNames = j.at("geo-tgeo-sensitive-names");
    vol.sensitiveAxes = j.at("geo-tgeo-sensitive-axes");
    vol.rRange = j.at("geo-tgeo-layer-r-ranges");
    vol.zRange = j.at("geo-tgeo-layer-z-ranges");
    vol.splitTolR = j.at("geo-tgeo-layer-r-split");
    vol.splitTolZ = j.at("geo-tgeo-layer-z-split");
    // Set binning manually
    vol.binning0 = j.at("geo-tgeo-binning0");
    vol.binning1 = j.at("geo-tgeo-binning1");

    vol.cylinderDiscSplit = j.at("geo-tgeo-cyl-disc-split");
    if (vol.cylinderDiscSplit)
    {
        Acts::TGeoCylinderDiscSplitter::Config cdConfig = j.at("Splitters").at("CylinderDisk");
        vol.cylinderNZSegments = cdConfig.cylinderLongitudinalSegments;
        vol.cylinderNPhiSegments = cdConfig.cylinderPhiSegments;
        vol.discNRSegments = cdConfig.discRadialSegments;
        vol.discNPhiSegments = cdConfig.discPhiSegments;
    }
    vol.itkModuleSplit = false;
}

void to_json(nlohmann::json& j, const TGeoConfig::Volume& vol) {
    j["geo-tgeo-volume-name"] = vol.name;

    j["geo-tgeo-sfbin-r-tolerance"] = vol.binToleranceR;
    j["geo-tgeo-sfbin-z-tolerance"] = vol.binToleranceZ;
    j["geo-tgeo-sfbin-phi-tolerance"] = vol.binTolerancePhi;

    j["geo-tgeo-volume-layers"] = vol.layers;
    j["geo-tgeo-subvolume-names"] = vol.subVolumeName;
    j["geo-tgeo-sensitive-names"] = vol.sensitiveNames;
    j["geo-tgeo-sensitive-axes"] = vol.sensitiveAxes;
    j["geo-tgeo-layer-r-ranges"] = vol.rRange;
    j["geo-tgeo-layer-z-ranges"] = vol.zRange;
    j["geo-tgeo-layer-r-split"] = vol.splitTolR;
    j["geo-tgeo-layer-z-split"] = vol.splitTolZ;
    j["geo-tgeo-binning0"] = vol.binning0;
    j["geo-tgeo-binning1"] = vol.binning1;

    j["geo-tgeo-cyl-disc-split"] = vol.cylinderDiscSplit;
    j["geo-tgeo-itk-module-split"] = vol.itkModuleSplit;

    Acts::TGeoCylinderDiscSplitter::Config cdConfig;
    cdConfig.cylinderLongitudinalSegments = vol.cylinderNZSegments;
    cdConfig.cylinderPhiSegments = vol.cylinderNPhiSegments;
    cdConfig.discRadialSegments = vol.discNRSegments;
    cdConfig.discPhiSegments = vol.discNPhiSegments;
    j["Splitters"]["CylinderDisk"] = cdConfig;
}

/// @brief Function that constructs a set of layer builder configs from a central @c TGeoDetector config.
///
/// @param config The input config
/// @return Vector of layer builder configs
std::vector<Acts::TGeoLayerBuilder::Config> makeLayerBuilderConfigs(
    const TGeoConfig& config, const Acts::Logger& logger)
{
    std::vector<Acts::TGeoLayerBuilder::Config> detLayerConfigs;

    // iterate over all configured detector volumes
    for (const auto& volume : config.volumes)
    {
        Acts::TGeoLayerBuilder::Config layerBuilderConfig;
        layerBuilderConfig.configurationName = volume.name;
        layerBuilderConfig.unit = config.unitScalor;
        layerBuilderConfig.elementFactory = config.elementFactory;

        // configure surface autobinning
        std::vector<std::pair<double, double>> binTolerances(
            static_cast<std::size_t>(Acts::binValues), {0., 0.});
        binTolerances[Acts::binR]   = {volume.binToleranceR.lower.value_or(0.),
                                       volume.binToleranceR.upper.value_or(0.)};
        binTolerances[Acts::binZ]   = {volume.binToleranceZ.lower.value_or(0.),
                                       volume.binToleranceZ.upper.value_or(0.)};
        binTolerances[Acts::binPhi] = {volume.binTolerancePhi.lower.value_or(0.),
                                       volume.binTolerancePhi.upper.value_or(0.)};

        layerBuilderConfig.autoSurfaceBinning = true;
        layerBuilderConfig.surfaceBinMatcher = Acts::SurfaceBinningMatcher(binTolerances);

        // loop over the negative/central/positive layer configurations
        for (auto ncp : { TGeoConfig::Negative,
                          TGeoConfig::Central,
                          TGeoConfig::Positive,} )
        {
            if (!volume.layers.at(ncp)) { continue; }
            
            Acts::TGeoLayerBuilder::LayerConfig lConfig;
            lConfig.volumeName = volume.subVolumeName.at(ncp);
            lConfig.sensorNames = volume.sensitiveNames.at(ncp);
            lConfig.localAxes = volume.sensitiveAxes.at(ncp);
            lConfig.envelope = {config.layerEnvelopeR, config.layerEnvelopeR};

            auto rR = volume.rRange.at(ncp);
            auto rMin = rR.lower.value_or(0.);
            auto rMax = rR.upper.value_or(std::numeric_limits<double>::max());
            auto zR = volume.zRange.at(ncp);
            auto zMin = zR.lower.value_or(-std::numeric_limits<double>::max());
            auto zMax = zR.upper.value_or(std::numeric_limits<double>::max());
            lConfig.parseRanges =
            {
                {Acts::binR, {rMin, rMax}},
                {Acts::binZ, {zMin, zMax}},
            };

            // Fill the layer splitting parameters in r/z
            auto str = volume.splitTolR.at(ncp);
            auto stz = volume.splitTolZ.at(ncp);
            if (0 < str) { lConfig.splitConfigs.emplace_back(Acts::binR, str); }
            if (0 < stz) { lConfig.splitConfigs.emplace_back(Acts::binZ, stz); }
            lConfig.binning0 = volume.binning0.at(ncp);
            lConfig.binning1 = volume.binning1.at(ncp);

            layerBuilderConfig.layerConfigurations[ncp].push_back(lConfig);
        }

        // Perform splitting of cylinders and discs
        if (volume.cylinderDiscSplit)
        {
            Acts::TGeoCylinderDiscSplitter::Config cdsConfig;
            cdsConfig.cylinderPhiSegments = volume.cylinderNPhiSegments;
            cdsConfig.cylinderLongitudinalSegments = volume.cylinderNZSegments;
            cdsConfig.discPhiSegments = volume.discNPhiSegments;
            cdsConfig.discRadialSegments = volume.discNRSegments;
            layerBuilderConfig.detectorElementSplitter =
            std::make_shared<const Acts::TGeoCylinderDiscSplitter>(cdsConfig,
                logger.clone("TGeoCylinderDiscSplitter", config.layerLogLevel));
        }

        detLayerConfigs.push_back(layerBuilderConfig);
    }

    return detLayerConfigs;
}

/// @brief Function to build the generic tracking geometry from a TGeo object.
///
/// @param TGeo_ROOTFilePath is the TGeo ROOT file path
/// @param TGeoConfig_jFilePath is the TGeo configuration file path
/// @param MaterialMap_jFilePath is the material map file path
/// @param logger is the logger object
/// @return a shared pointer to the tracking geometry
std::shared_ptr<const Acts::TrackingGeometry> buildTGeoDetector(
    const Acts::GeometryContext& context,
    std::vector<std::shared_ptr<const Acts::TGeoDetectorElement>>& detElementStore,
    const std::string& TGeo_ROOTFilePath,
    const std::string& TGeoConfig_jFilePath,
    const std::string& MaterialMap_jFilePath,
    const Acts::Logger& logger)
{
    if (TGeo_ROOTFilePath.empty() | TGeoConfig_jFilePath.empty() |
        MaterialMap_jFilePath.empty()) { return nullptr; }

    TGeoConfig config;
    nlohmann::json djson;
    const Acts::MaterialMapJsonConverter::Config m_converter;

    // read input files
    config.fileName = TGeo_ROOTFilePath;
    std::shared_ptr<const Acts::IMaterialDecorator> mdecorator = std::make_shared<const Acts::JsonMaterialDecorator>(m_converter, MaterialMap_jFilePath, Acts::Logging::Level::INFO);
    std::ifstream infile(TGeoConfig_jFilePath, std::ifstream::in | std::ifstream::binary);

    // ------------------------------------------
    //   read TGeo Layer Builder Configs File
    // ------------------------------------------
    infile >> djson;
    config.unitScalor = djson["geo-tgeo-unit-scalor"];
    config.buildBeamPipe = djson["geo-tgeo-build-beampipe"];
    if (config.buildBeamPipe)
    {
        const auto beamPipeParameters = djson["geo-tgeo-beampipe-parameters"].get<std::array<double, 3>>();
        config.beamPipeRadius = beamPipeParameters[0];
        config.beamPipeHalflengthZ = beamPipeParameters[1];
        config.beamPipeLayerThickness = beamPipeParameters[2];
    }

    // Fill nested volume configs
    for (const auto& volume : djson["Volumes"])
    {
        auto& vol = config.volumes.emplace_back();
        vol = volume;
    }

    // ------------------------------------------
    //         logger Configaration
    // ------------------------------------------
    Acts::SurfaceArrayCreator::Config sacConfig;
    auto surfaceArrayCreator = std::make_shared<const Acts::SurfaceArrayCreator>(
        sacConfig, logger.clone("SurfaceArrayCreator", config.surfaceLogLevel));
    // configure the proto layer helper
    Acts::ProtoLayerHelper::Config plhConfig;
    auto protoLayerHelper = std::make_shared<const Acts::ProtoLayerHelper>(
        plhConfig, logger.clone("ProtoLayerHelper", config.layerLogLevel));
    // configure the layer creator that uses the surface array creator
    Acts::LayerCreator::Config lcConfig;
    lcConfig.surfaceArrayCreator = surfaceArrayCreator;
    auto layerCreator = std::make_shared<const Acts::LayerCreator>(
        lcConfig, logger.clone("LayerCreator", config.layerLogLevel));
    // configure the layer array creator
    Acts::LayerArrayCreator::Config lacConfig;
    auto layerArrayCreator = std::make_shared<const Acts::LayerArrayCreator>(
        lacConfig, logger.clone("LayerArrayCreator", config.layerLogLevel));
    // tracking volume array creator
    Acts::TrackingVolumeArrayCreator::Config tvacConfig;
    auto tVolumeArrayCreator = 
        std::make_shared<const Acts::TrackingVolumeArrayCreator>( tvacConfig,
            logger.clone("TrackingVolumeArrayCreator", config.volumeLogLevel));

    // configure the cylinder volume helper
    Acts::CylinderVolumeHelper::Config cvhConfig;
    cvhConfig.layerArrayCreator = layerArrayCreator;
    cvhConfig.trackingVolumeArrayCreator = tVolumeArrayCreator;
    auto cylinderVolumeHelper =
        std::make_shared<const Acts::CylinderVolumeHelper>(cvhConfig,
            logger.clone("CylinderVolumeHelper", config.volumeLogLevel));

    // ------------------------------------------
    //   logger Configaration
    // ------------------------------------------
    // list the volume builders
    std::list<std::shared_ptr<const Acts::ITrackingVolumeBuilder>> volumeBuilders;

    // Create a beam pipe if configured to do so
    if (config.buildBeamPipe)
    {
        /// configure the beam pipe layer builder
        Acts::PassiveLayerBuilder::Config bplConfig;
        bplConfig.layerIdentification = "BeamPipe";
        bplConfig.centralLayerRadii = {config.beamPipeRadius};
        bplConfig.centralLayerHalflengthZ = {config.beamPipeHalflengthZ};
        bplConfig.centralLayerThickness = {config.beamPipeLayerThickness};
        auto beamPipeBuilder = std::make_shared<const Acts::PassiveLayerBuilder>(
            bplConfig, logger.clone("BeamPipeLayerBuilder", config.layerLogLevel));
        // create the volume for the beam pipe
        Acts::CylinderVolumeBuilder::Config bpvConfig;
        bpvConfig.trackingVolumeHelper = cylinderVolumeHelper;
        bpvConfig.volumeName = "BeamPipe";
        bpvConfig.layerBuilder = beamPipeBuilder;
        bpvConfig.layerEnvelopeR = {config.beamPipeEnvelopeR, config.beamPipeEnvelopeR};
        bpvConfig.buildToRadiusZero = true;
        auto beamPipeVolumeBuilder =
            std::make_shared<const Acts::CylinderVolumeBuilder>(bpvConfig,
                logger.clone("BeamPipeVolumeBuilder", config.volumeLogLevel));
        // add to the list of builders
        volumeBuilders.push_back(beamPipeVolumeBuilder);
    }

    // Import the file from
    TGeoManager::Import(config.fileName.c_str());
    auto layerBuilderConfigs = makeLayerBuilderConfigs(config, logger);
    // Remember the layer builders to collect the detector elements
    std::vector<std::shared_ptr<const Acts::TGeoLayerBuilder>> tgLayerBuilders;
    for (auto& lbc : layerBuilderConfigs)
    {
        std::shared_ptr<const Acts::LayerCreator> layerCreatorLB = nullptr;
        if (lbc.autoSurfaceBinning)
        {
            // Configure surface array creator (optionally) per layer builder
            // (in order to configure them to work appropriately)
            Acts::SurfaceArrayCreator::Config sacConfigLB;
            sacConfigLB.surfaceMatcher = lbc.surfaceBinMatcher;
            auto surfaceArrayCreatorLB =
                std::make_shared<const Acts::SurfaceArrayCreator>(sacConfigLB,
                    logger.clone(lbc.configurationName + "SurfaceArrayCreator", config.surfaceLogLevel));
            // configure the layer creator that uses the surface array creator
            Acts::LayerCreator::Config lcConfigLB;
            lcConfigLB.surfaceArrayCreator = surfaceArrayCreatorLB;
            layerCreatorLB = std::make_shared<const Acts::LayerCreator>(lcConfigLB,
                logger.clone(lbc.configurationName + "LayerCreator", config.layerLogLevel));
        }

        // Configure the proto layer helper
        Acts::ProtoLayerHelper::Config plhConfigLB;
        auto protoLayerHelperLB = std::make_shared<const Acts::ProtoLayerHelper>( plhConfigLB,
            logger.clone(lbc.configurationName + "ProtoLayerHelper", config.layerLogLevel));

        lbc.layerCreator =
            (layerCreatorLB != nullptr) ? layerCreatorLB : layerCreator;
        lbc.protoLayerHelper =
            (protoLayerHelperLB != nullptr) ? protoLayerHelperLB : protoLayerHelper;

        auto layerBuilder = std::make_shared<const Acts::TGeoLayerBuilder>( lbc,
            logger.clone(lbc.configurationName + "LayerBuilder", config.layerLogLevel));

        // remember the layer builder
        tgLayerBuilders.push_back(layerBuilder);

        // build the pixel volume
        Acts::CylinderVolumeBuilder::Config volumeConfig;
        volumeConfig.trackingVolumeHelper = cylinderVolumeHelper;
        volumeConfig.volumeName = lbc.configurationName;
        volumeConfig.buildToRadiusZero = volumeBuilders.empty();
        volumeConfig.layerEnvelopeR = {config.layerEnvelopeR, config.layerEnvelopeR};
        auto ringLayoutConfiguration =
            [&](const std::vector<Acts::TGeoLayerBuilder::LayerConfig>& lConfigs) -> void
            {
                for (const auto& lcfg : lConfigs)
                {
                    for (const auto& scfg : lcfg.splitConfigs)
                    {
                        if (scfg.first == Acts::binR && scfg.second > 0.)
                        {
                            volumeConfig.ringTolerance = std::max(volumeConfig.ringTolerance, scfg.second);
                            volumeConfig.checkRingLayout = true;
                        }
                    }
                }
            };
        ringLayoutConfiguration(lbc.layerConfigurations[0]);
        ringLayoutConfiguration(lbc.layerConfigurations[2]);
        volumeConfig.layerBuilder = layerBuilder;
        auto volumeBuilder = std::make_shared<const Acts::CylinderVolumeBuilder>( volumeConfig,
            logger.clone(lbc.configurationName + "VolumeBuilder", config.volumeLogLevel));
        // add to the list of builders
        volumeBuilders.push_back(volumeBuilder);
    }

    //-------------------------------------------------------------------------------------
    // create the tracking geometry
    Acts::TrackingGeometryBuilder::Config tgConfig;
    // Add the builders
    tgConfig.materialDecorator = std::move(mdecorator);
    tgConfig.geometryIdentifierHook = config.geometryIdentifierHook;

    for (auto& vb : volumeBuilders)
    {
        tgConfig.trackingVolumeBuilders.push_back(
            [=](const auto& gcontext, const auto& inner, const auto&)
            { return vb->trackingVolume(gcontext, inner); });
    }
    // Add the helper
    tgConfig.trackingVolumeHelper = cylinderVolumeHelper;
    auto cylinderGeometryBuilder = std::make_shared<const Acts::TrackingGeometryBuilder>(
        tgConfig, logger.clone("TrackerGeometryBuilder", config.volumeLogLevel));
    // get the geometry
    auto trackingGeometry = cylinderGeometryBuilder->trackingGeometry(context);
    // collect the detector element store
    for (auto& lBuilder : tgLayerBuilders)
    {
        auto detElements = lBuilder->detectorElements();
        detElementStore.insert(detElementStore.begin(), detElements.begin(), detElements.end());
    }

    /// return the tracking geometry
    return trackingGeometry;
}