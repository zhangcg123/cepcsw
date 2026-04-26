#ifndef RecActsTracking_H
#define RecActsTracking_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <filesystem>

#include "k4FWCore/DataHandle.h"
#include "GaudiKernel/Algorithm.h"
#include "DD4hep/Detector.h"
#include "DDRec/DetectorData.h"
#include "DDRec/ISurface.h"
#include "DDRec/SurfaceManager.h"
#include "DDRec/Vector3D.h"

#include "UTIL/ILDConf.h"
#include "GaudiKernel/NTuple.h"
#include "DetInterface/IGeomSvc.h"

// gear
#include "GearSvc/IGearSvc.h"
#include <gear/GEAR.h>
#include <gear/GearMgr.h>
#include <gear/GearParameters.h>
#include <gear/VXDLayerLayout.h>
#include <gear/VXDParameters.h>
#include "gear/FTDLayerLayout.h"
#include "gear/FTDParameters.h"
#include <gear/BField.h>

// edm4hep
#include "edm4hep/MCParticle.h"
#include "edm4hep/Track.h"
#include "edm4hep/MutableTrack.h"
// #include "edm4hep/TrackerHit.h"
// #include "edm4hep/SimTrackerHit.h"
#include "edm4hep/TrackState.h"
#include "edm4hep/EventHeaderCollection.h"
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackerHitCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"

// acts
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Definitions/Direction.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"

#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/ProxyAccessor.hpp"
#include "Acts/EventData/SpacePointData.hpp"
#include <Acts/EventData/Measurement.hpp>
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/EventData/TrackContainer.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/EventData/ParticleHypothesis.hpp"

#include "Acts/Propagator/AbortList.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/MaterialInteractor.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/StandardAborters.hpp"

#include "Acts/Geometry/Extent.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include <Acts/Geometry/GeometryContext.hpp>

#include "Acts/Seeding/BinnedGroup.hpp"
#include "Acts/Seeding/EstimateTrackParamsFromSeed.hpp"
#include "Acts/Seeding/InternalSpacePoint.hpp"
#include "Acts/Seeding/SeedFilter.hpp"
#include "Acts/Seeding/SeedFilterConfig.hpp"
#include "Acts/Seeding/SeedFinder.hpp"
#include "Acts/Seeding/SeedFinderConfig.hpp"
#include "Acts/Seeding/SpacePointGrid.hpp"

#include "Acts/TrackFinding/CombinatorialKalmanFilter.hpp"
#include "Acts/TrackFitting/GainMatrixSmoother.hpp"
#include "Acts/TrackFitting/GainMatrixUpdater.hpp"
#include "Acts/TrackFitting/KalmanFitter.hpp"

#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/Surface.hpp"

#include "Acts/Utilities/BinningType.hpp"
#include "Acts/Utilities/Delegate.hpp"
#include "Acts/Utilities/Grid.hpp"
#include "Acts/Utilities/GridBinFinder.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Utilities/Enumerate.hpp"
#include "Acts/Utilities/TrackHelpers.hpp"

// local include
#include "utils/TGeoDetector.hpp"
#include "utils/CKFhelper.hpp"
#include "utils/MagneticField.hpp"

namespace gear{
  class GearMgr;
}

namespace dd4hep {
    namespace DDSegmentation {
        class BitFieldCoder;
    }
    namespace rec{
        class ISurface;
    }
}

// Seeding: Construct track seeds from space points.
// config for seed finding
struct SeedingConfig
{
    /// Input space point collections.
    ///
    /// We allow multiple space point collections to allow different parts of
    /// the detector to use different algorithms for space point construction,
    /// e.g. single-hit space points for pixel-like detectors or double-hit
    /// space points for strip-like detectors.
    std::vector<std::string> inputSpacePoints;

    /// Output track seed collection.
    std::string outputSeeds;


    Acts::SeedFilterConfig seedFilterConfig;
    Acts::SeedFinderConfig<SimSpacePoint> seedFinderConfig;
    Acts::CylindricalSpacePointGridConfig gridConfig;
    Acts::CylindricalSpacePointGridOptions gridOptions;
    Acts::SeedFinderOptions seedFinderOptions;

    // allow for different values of rMax in gridConfig and seedFinderConfig
    bool allowSeparateRMax = false;

    // vector containing the map of z bins in the top and bottom layers
    std::vector<std::pair<int, int>> zBinNeighborsTop;
    std::vector<std::pair<int, int>> zBinNeighborsBottom;

    // number of phiBin neighbors at each side of the current bin that will be
    // used to search for SPs
    int numPhiNeighbors = 1;
};

class RecActsTracking : public Algorithm
{

    public :

        RecActsTracking(const std::string& name, ISvcLocator* svcLoc);

        virtual StatusCode initialize();

        virtual StatusCode execute();

        virtual StatusCode finalize();

    private :

        // Input collections
        DataHandle<edm4hep::TrackerHitCollection> _inVTXTrackHdl{"VXDTrackerHits", Gaudi::DataHandle::Reader, this};
        DataHandle<edm4hep::TrackerHitCollection> _inSITTrackHdl{"SITTrackerHits", Gaudi::DataHandle::Reader, this};
        DataHandle<edm4hep::TrackerHitCollection> _inFTDTrackHdl{"FTDTrackerHits", Gaudi::DataHandle::Reader, this};

        DataHandle<edm4hep::SimTrackerHitCollection> _inVTXColHdl{"VXDCollection", Gaudi::DataHandle::Reader, this};
        DataHandle<edm4hep::SimTrackerHitCollection> _inSITColHdl{"SITCollection", Gaudi::DataHandle::Reader, this};
        DataHandle<edm4hep::SimTrackerHitCollection> _inFTDColHdl{"FTDCollection", Gaudi::DataHandle::Reader, this};

        DataHandle<edm4hep::MCParticleCollection> _inMCColHdl{"MCParticle", Gaudi::DataHandle::Reader, this};

        // Output collections
        DataHandle<edm4hep::TrackCollection> _outColHdl{"ACTSSiTracks", Gaudi::DataHandle::Writer, this};

        // properties
        Gaudi::Property<std::string> TGeo_path{this, "TGeoFile"};
        Gaudi::Property<std::string> TGeo_config_path{this, "TGeoConfigFile"};
        Gaudi::Property<std::string> MaterialMap_path{this, "MaterialMapFile"};
        Gaudi::Property<std::string> m_particle{this, "AssumeParticle"};
        Gaudi::Property<double> m_field{this, "Field", 3.0}; // tesla
        Gaudi::Property<double> onSurfaceTolerance{this, "onSurfaceTolerance", 1e-2}; // mm
        Gaudi::Property<double> eps{this, "eps", 1e-5}; // mm
        Gaudi::Property<bool> ExtendSeedRange{this, "ExtendSeedRange", false};

        // seed finder config
        Gaudi::Property<double> SeedDeltaRMin{this, "SeedDeltaRMin", 4}; // mm
        Gaudi::Property<double> SeedDeltaRMax{this, "SeedDeltaRMax", 13}; // mm
        Gaudi::Property<double> SeedRMax{this, "SeedRMax", 30}; // mm
        Gaudi::Property<double> SeedRMin{this, "SeedRMin", 10}; // mm
        Gaudi::Property<double> SeedImpactMax{this, "SeedImpactMax", 3}; // mm
        Gaudi::Property<double> SeedRMinMiddle{this, "SeedRMinMiddle", 14}; // mm
        Gaudi::Property<double> SeedRMaxMiddle{this, "SeedRMaxMiddle", 24}; // mm
        Gaudi::Property<std::vector<double>> initialVarInflation{this, "initialVarInflation", {1, 1, 20, 20, 20, 20}};

        // CKF config
        Gaudi::Property<double> CKFchi2Cut{this, "CKFchi2Cut", std::numeric_limits<double>::max()};
        Gaudi::Property<std::size_t> numMeasurementsCutOff{this, "numMeasurementsCutOff", 1u};
        Gaudi::Property<bool> CKFtwoWay{this, "CKFtwoWay", true};
        
        SmartIF<IGeomSvc> m_geosvc;
        SmartIF<IChronoStatSvc> chronoStatSvc;
        dd4hep::DDSegmentation::BitFieldCoder *vxd_decoder;
        dd4hep::DDSegmentation::BitFieldCoder *sit_decoder;
        dd4hep::DDSegmentation::BitFieldCoder *ftd_decoder;
        const dd4hep::rec::SurfaceMap* m_vtx_surfaces;
        const dd4hep::rec::SurfaceMap* m_sit_surfaces;
        const dd4hep::rec::SurfaceMap* m_ftd_surfaces;

        // configs to build acts geometry
        Acts::GeometryContext geoContext;
        Acts::MagneticFieldContext magFieldContext;  
        Acts::CalibrationContext calibContext;
        std::string TGeo_ROOTFilePath;
        std::string TGeoConfig_jFilePath;
        std::string MaterialMap_jFilePath;
        std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry;
        std::vector<std::shared_ptr<const Acts::TGeoDetectorElement>> detElementStore;

        // Store the space points & measurements & sourcelinks **per event**
        // * only store the VXD tracker hits to the SpacePointPtrs
        std::vector<const SimSpacePoint*> SpacePointPtrs;
        IndexSourceLinkContainer sourceLinks;
        std::vector<::Acts::BoundVariantMeasurement> measurements;
        std::vector<::Acts::BoundTrackParameters> initialParameters;
        SimSeedContainer Selected_Seeds;

        // seed finder
        SeedingConfig seed_cfg;
        std::unique_ptr<const Acts::GridBinFinder<2ul>> m_bottomBinFinder;
        std::unique_ptr<const Acts::GridBinFinder<2ul>> m_topBinFinder;
        Acts::SeedFinder<SimSpacePoint, Acts::CylindricalSpacePointGrid<SimSpacePoint>> m_seedFinder;

        int InitialiseVTX();
        int InitialiseSIT();
        int InitialiseFTD();
        const dd4hep::rec::ISurface* getISurface(edm4hep::TrackerHit* hit);
        const Acts::GeometryIdentifier getVTXGid(uint64_t cellid);
        const Acts::GeometryIdentifier getSITGid(uint64_t cellid);

        // utils
        int _nEvt;
        const double _FCT = 2.99792458E-4;
        Acts::Vector3 acts_field_value = Acts::Vector3(0., 0., 3*_FCT); // tesla
        // Acts::Vector3 acts_field_value = Acts::Vector3(0., 0., 3); // tesla
        std::shared_ptr<const Acts::MagneticFieldProvider> magneticField
            = std::make_shared<Acts::ConstantBField>(acts_field_value);
        double bFieldMin = 0.3 * _FCT * Acts::UnitConstants::T;  // tesla
        // double bFieldMin = 0.1 * Acts::UnitConstants::T;  // tesla
        const Acts::Vector3 globalFakeMom{1.e6, 1.e6, 1.e6};
        const std::array<Acts::BoundIndices, 6> writeout_indices{
            Acts::BoundIndices::eBoundLoc0, Acts::BoundIndices::eBoundPhi,
            Acts::BoundIndices::eBoundQOverP, Acts::BoundIndices::eBoundLoc1,
            Acts::BoundIndices::eBoundTheta, Acts::BoundIndices::eBoundTime};

        // param estimate configuration
        double noTimeVarInflation = 100.;
        std::array<double, 6> initialSigmas = {
            5 * Acts::UnitConstants::um,
            5 * Acts::UnitConstants::um,
            2e-2 * Acts::UnitConstants::degree,
            2e-2 * Acts::UnitConstants::degree,
            1e-1 * Acts::UnitConstants::e / Acts::UnitConstants::GeV,
            1 * Acts::UnitConstants::s};
        std::array<double, 6> initialSimgaQoverPCoefficients = {
            0 * Acts::UnitConstants::mm / (Acts::UnitConstants::e * Acts::UnitConstants::GeV),
            0 * Acts::UnitConstants::mm / (Acts::UnitConstants::e * Acts::UnitConstants::GeV),
            7.1e-2 * Acts::UnitConstants::degree / (Acts::UnitConstants::e * Acts::UnitConstants::GeV),
            2.1e-2 * Acts::UnitConstants::degree / (Acts::UnitConstants::e * Acts::UnitConstants::GeV),
            6.4e-2,
            0 * Acts::UnitConstants::ns / (Acts::UnitConstants::e * Acts::UnitConstants::GeV)};
        // std::array<double, 6> initialVarInflation = {10., 10., 10., 10., 10., 10.};
        Acts::ParticleHypothesis particleHypothesis = Acts::ParticleHypothesis::muon();
        std::array<std::string, 8> particleNames = {"muon", "pion", "electron", "kaon", "proton", "photon", "geantino", "chargedgeantino"};
        // Acts::ParticleHypothesis particleHypothesis = Acts::ParticleHypothesis::chargedGeantino();

        // gid convert configuration
        std::vector<uint64_t> VXD_volume_ids{20, 21, 22, 23, 24};
        std::vector<uint64_t> SIT_volume_ids{28, 31, 34};
        std::vector<uint64_t> FTD_positive_volume_ids{29, 32, 35};
        std::vector<uint64_t> FTD_negative_volume_ids{27, 7, 2};
        std::vector<uint64_t> SIT_module_nums{7, 10, 14};

        // CKF configuration
        // Acts::MeasurementSelector::Config measurementSelectorCfg;
        std::shared_ptr<TrackFinderFunction> findTracks;
        std::optional<Acts::TrackSelector> m_trackSelector;
        std::optional<std::variant<Acts::TrackSelector::Config, Acts::TrackSelector::EtaBinnedConfig>> trackSelectorCfg = std::nullopt;
        mutable std::atomic<std::size_t> m_nTotalSeeds{0};
        mutable std::atomic<std::size_t> m_nDeduplicatedSeeds{0};
        mutable std::atomic<std::size_t> m_nFailedSeeds{0};
        mutable std::atomic<std::size_t> m_nFailedSmoothing{0};
        mutable std::atomic<std::size_t> m_nFailedExtrapolation{0};
        mutable std::atomic<std::size_t> m_nFoundTracks{0};
        mutable std::atomic<std::size_t> m_nSelectedTracks{0};
        mutable std::atomic<std::size_t> m_nStoppedBranches{0};
        // layer hits, VXD (0-5) & SIT (6-8)
        std::array<std::atomic<size_t>, 9> m_nLayerHits{0, 0, 0, 0, 0, 0, 0, 0, 0};
        std::array<std::atomic<size_t>, 6> m_nRec_VTX{0, 0, 0, 0, 0, 0};
        std::array<std::atomic<size_t>, 3> m_nRec_SIT{0, 0, 0};
        std::array<std::atomic<size_t>, 3> m_nRec_FTD{0, 0, 0};
        std::array<std::atomic<size_t>, 9> m_n0EventHits{0, 0, 0, 0, 0, 0, 0, 0, 0};
        std::array<std::atomic<size_t>, 9> m_n1EventHits{0, 0, 0, 0, 0, 0, 0, 0, 0};
        std::array<std::atomic<size_t>, 9> m_n2EventHits{0, 0, 0, 0, 0, 0, 0, 0, 0};
        std::array<std::atomic<size_t>, 9> m_n3EventHits{0, 0, 0, 0, 0, 0, 0, 0, 0};

        mutable tbb::combinable<Acts::VectorMultiTrajectory::Statistics> m_memoryStatistics{[]() {
            auto mtj = std::make_shared<Acts::VectorMultiTrajectory>();
            return mtj->statistics();
        }};
        bool twoWay = false; /// Run finding in two directions
        Acts::TrackExtrapolationStrategy extrapolationStrategy = Acts::TrackExtrapolationStrategy::firstOrLast; /// Extrapolation strategy
        unsigned int maxSteps = 100000;
};

#endif  // RecActsTracking_H
