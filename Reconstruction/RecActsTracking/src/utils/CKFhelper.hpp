#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Definitions/Direction.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"

#include "Acts/EventData/ProxyAccessor.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/SourceLink.hpp"
#include "Acts/EventData/TrackContainer.hpp"
#include "Acts/EventData/TrackProxy.hpp"
#include <Acts/EventData/Measurement.hpp>
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"

#include "ActsFatras/Digitization/Segmentizer.hpp"

#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Geometry/GeometryContext.hpp"

#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/Surface.hpp"

#include "Acts/Propagator/AbortList.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/MaterialInteractor.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/StandardAborters.hpp"


#include "Acts/TrackFinding/CombinatorialKalmanFilter.hpp"
#include "Acts/TrackFinding/MeasurementSelector.hpp"
#include "Acts/TrackFinding/TrackSelector.hpp"
#include "Acts/TrackFitting/GainMatrixSmoother.hpp"
#include "Acts/TrackFitting/GainMatrixUpdater.hpp"
#include "Acts/TrackFitting/KalmanFitter.hpp"

#include "Acts/Utilities/Logger.hpp"
#include "Acts/Utilities/Result.hpp"
#include "Acts/Utilities/TrackHelpers.hpp"
#include "Acts/Utilities/Delegate.hpp"
#include "Acts/Utilities/Enumerate.hpp"
#include "Acts/Utilities/TrackHelpers.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"

#include <atomic>
#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <cmath>
#include <ostream>
#include <stdexcept>
#include <system_error>
#include <unordered_map>
#include <utility>

#include <tbb/combinable.h>
#include <boost/functional/hash.hpp>

#include "SimSpacePoint.hpp"

namespace Acts
{
    class MagneticFieldProvider;
    class TrackingGeometry;
}

using Updater = Acts::GainMatrixUpdater;
using Smoother = Acts::GainMatrixSmoother;
using Stepper = Acts::EigenStepper<>;
using Navigator = Acts::Navigator;
using Propagator = Acts::Propagator<Stepper, Navigator>;
using CKF = Acts::CombinatorialKalmanFilter<Propagator, Acts::VectorMultiTrajectory>;

// track container types
using TrackContainer = Acts::TrackContainer<Acts::VectorTrackContainer, Acts::VectorMultiTrajectory, std::shared_ptr>;
using ConstTrackContainer = Acts::TrackContainer<Acts::ConstVectorTrackContainer, Acts::ConstVectorMultiTrajectory, std::shared_ptr>;
using TrackParameters = ::Acts::BoundTrackParameters;
using TrackParametersContainer = std::vector<TrackParameters>;
using TrackIndexType = TrackContainer::IndexType;
using TrackProxy = TrackContainer::TrackProxy;
using ConstTrackProxy = ConstTrackContainer::ConstTrackProxy;

// track finder types
using TrackFinderOptions = Acts::CombinatorialKalmanFilterOptions<IndexSourceLinkAccessor::Iterator, Acts::VectorMultiTrajectory>;
using TrackFinderResult = Acts::Result<std::vector<TrackContainer::TrackProxy>>;

// measurement types
using Measurement = ::Acts::BoundVariantMeasurement;
using MeasurementContainer = std::vector<Measurement>;

class TrackFinderFunction
{
public:
    virtual ~TrackFinderFunction() = default;
    virtual TrackFinderResult operator()(const TrackParameters&, const TrackFinderOptions&, TrackContainer&) const = 0;
};

static std::shared_ptr<TrackFinderFunction> makeTrackFinderFunction(
    std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry,
    std::shared_ptr<const Acts::MagneticFieldProvider> magneticField,
    const Acts::Logger& logger);

struct TrackFinderFunctionImpl : public TrackFinderFunction {
    CKF trackFinder;
    TrackFinderFunctionImpl(CKF&& f) : trackFinder(std::move(f)) {}
    TrackFinderResult operator()( const TrackParameters& initialParameters,
                                  const TrackFinderOptions& options,
                                  TrackContainer& tracks) const override
    {
        return trackFinder.findTracks(initialParameters, options, tracks);
    };
};

std::shared_ptr<TrackFinderFunction> makeTrackFinderFunction(
    std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry,
    std::shared_ptr<const Acts::MagneticFieldProvider> magneticField)
{
  Stepper stepper(magneticField);
  Navigator::Config cfg{trackingGeometry};
  cfg.resolvePassive = false;
  cfg.resolveMaterial = true;
  cfg.resolveSensitive = true;
  Navigator navigator(cfg);
  Propagator propagator(std::move(stepper), std::move(navigator));
  CKF trackFinder(std::move(propagator));

  return std::make_shared<TrackFinderFunctionImpl>(std::move(trackFinder));
}

struct Cluster
{
  using Cell = ActsFatras::Segmentizer::ChannelSegment;
  std::size_t sizeLoc0 = 0;
  std::size_t sizeLoc1 = 0;
  std::vector<Cell> channels;
};

using ClusterContainer = std::vector<Cluster>;

/// Abstract base class for measurement-based calibration
class MeasurementCalibrator
{
    public:
        virtual void calibrate(
            const MeasurementContainer& measurements, const ClusterContainer* clusters,
            const Acts::GeometryContext& gctx, const Acts::CalibrationContext& cctx, 
            const Acts::SourceLink& sourceLink,
            Acts::VectorMultiTrajectory::TrackStateProxy& trackState) const = 0;
        virtual ~MeasurementCalibrator() = default;
        virtual bool needsClusters() const { return false; }
};

// Calibrator to convert an index source link to a measurement as-is
class PassThroughCalibrator : public MeasurementCalibrator
{
    public:
        /// Find the measurement corresponding to the source link.
        ///
        /// @tparam parameters_t Track parameters type
        /// @param gctx The geometry context (unused)
        /// @param trackState The track state to calibrate
        void calibrate(
            const MeasurementContainer& measurements,
            const ClusterContainer* clusters, const Acts::GeometryContext& gctx,
            const Acts::CalibrationContext& cctx, const Acts::SourceLink& sourceLink,
            Acts::VectorMultiTrajectory::TrackStateProxy& trackState) const override;
};

// Adapter class that wraps a MeasurementCalibrator to conform to the
// core ACTS calibration interface
class MeasurementCalibratorAdapter
{
    public:
        MeasurementCalibratorAdapter(const MeasurementCalibrator& calibrator,
                                     const MeasurementContainer& measurements,
                                     const ClusterContainer* clusters = nullptr);
        MeasurementCalibratorAdapter() = delete;
        
        void calibrate(const Acts::GeometryContext& gctx,
                       const Acts::CalibrationContext& cctx,
                       const Acts::SourceLink& sourceLink,
                       Acts::VectorMultiTrajectory::TrackStateProxy trackState) const;

    private:
        const MeasurementCalibrator& m_calibrator;
        const MeasurementContainer& m_measurements;
        const ClusterContainer* m_clusters;
};

void PassThroughCalibrator::calibrate(
    const MeasurementContainer& measurements, const ClusterContainer* /*clusters*/,
    const Acts::GeometryContext& /*gctx*/, const Acts::CalibrationContext& /*cctx*/,
    const Acts::SourceLink& sourceLink, Acts::VectorMultiTrajectory::TrackStateProxy& trackState) const
{
    trackState.setUncalibratedSourceLink(sourceLink);
    const IndexSourceLink& idxSourceLink = sourceLink.get<IndexSourceLink>();

    assert((idxSourceLink.index() < measurements.size()) &&
            "Source link index is outside the container bounds");

    std::visit(
        [&trackState](const auto& meas) { trackState.setCalibrated(meas); },
        measurements[idxSourceLink.index()]);
}

MeasurementCalibratorAdapter::MeasurementCalibratorAdapter(
    const MeasurementCalibrator& calibrator, const MeasurementContainer& measurements,
    const ClusterContainer* clusters) : m_calibrator{calibrator}, m_measurements{measurements}, m_clusters{clusters} {}

void MeasurementCalibratorAdapter::calibrate(
    const Acts::GeometryContext& gctx, const Acts::CalibrationContext& cctx, const Acts::SourceLink& sourceLink,
    Acts::VectorMultiTrajectory::TrackStateProxy trackState) const
{
    return m_calibrator.calibrate(m_measurements, m_clusters, gctx, cctx, sourceLink, trackState);
}

// Specialize std::hash for SeedIdentifier
// This is required to use SeedIdentifier as a key in an `std::unordered_map`.
template <class T, std::size_t N>
struct std::hash<std::array<T, N>>
{
    std::size_t operator()(const std::array<T, N>& array) const
    {
        std::hash<T> hasher;
        std::size_t result = 0;
        for (auto&& element : array) { boost::hash_combine(result, hasher(element)); }
        return result;
    }
};

// Measurement selector for seed
class MeasurementSelector
{
    public:
        using Traj = Acts::VectorMultiTrajectory;
        explicit MeasurementSelector(Acts::MeasurementSelector selector)
            : m_selector(std::move(selector)) {}
        
        void setSeed(const std::optional<SimSeed>& seed) { m_seed = seed; }

        Acts::Result<std::pair<std::vector<Traj::TrackStateProxy>::iterator,
                               std::vector<Traj::TrackStateProxy>::iterator>>
        select(std::vector<Traj::TrackStateProxy>& candidates,
               bool& isOutlier, const Acts::Logger& logger) const
        {
            if (m_seed.has_value())
            {
                std::vector<Traj::TrackStateProxy> newCandidates;
                for (const auto& candidate : candidates)
                {
                    if (isSeedCandidate(candidate)) { newCandidates.push_back(candidate); }
                }

                if (!newCandidates.empty()) { candidates = std::move(newCandidates); }
            }

            return m_selector.select<Acts::VectorMultiTrajectory>(candidates, isOutlier, logger);
        }
    
    private:
        Acts::MeasurementSelector m_selector;
        std::optional<SimSeed> m_seed;

        bool isSeedCandidate(const Traj::TrackStateProxy& candidate) const
        {
            assert(candidate.hasUncalibratedSourceLink());
            const Acts::SourceLink& sourceLink = candidate.getUncalibratedSourceLink();
            for (const auto& sp : m_seed->sp())
            {
                for (const auto& sl : sp->sourceLinks())
                {
                    if (sourceLink.get<IndexSourceLink>() == sl.get<IndexSourceLink>()) { return true; }
                }
            }
            return false;
        }

}; // class MeasurementSelector

/// Source link indices of the bottom, middle, top measurements.
/// * In case of strip seeds only the first source link of the pair is used.
using SeedIdentifier = std::array<Index, 3>;

/// Build a seed identifier from a seed.
///
/// @param seed The seed to build the identifier from.
/// @return The seed identifier.
SeedIdentifier makeSeedIdentifier(const SimSeed& seed)
{
    SeedIdentifier result;
    for (const auto& [i, sp] : Acts::enumerate(seed.sp()))
    {
        const Acts::SourceLink& firstSourceLink = sp->sourceLinks().front();
        result.at(i) = firstSourceLink.get<IndexSourceLink>().index();
    }
    return result;
}

/// Visit all possible seed identifiers of a track.
///
/// @param track The track to visit the seed identifiers of.
/// @param visitor The visitor to call for each seed identifier.
template <typename Visitor>
void visitSeedIdentifiers(const TrackProxy& track, Visitor visitor)
{
    // first we collect the source link indices of the track states
    std::vector<Index> sourceLinkIndices;
    sourceLinkIndices.reserve(track.nMeasurements());
    for (const auto& trackState : track.trackStatesReversed()) 
    {
        if (!trackState.hasUncalibratedSourceLink()) { continue; }
        const Acts::SourceLink& sourceLink = trackState.getUncalibratedSourceLink();
        sourceLinkIndices.push_back(sourceLink.get<IndexSourceLink>().index());
    }

    // then we iterate over all possible triplets and form seed identifiers
    for (std::size_t i = 0; i < sourceLinkIndices.size(); ++i)
    {
        for (std::size_t j = i + 1; j < sourceLinkIndices.size(); ++j)
        {
            for (std::size_t k = j + 1; k < sourceLinkIndices.size(); ++k)
            {
                // Putting them into reverse order (k, j, i) to compensate for the `trackStatesReversed` above.
                visitor({sourceLinkIndices.at(k), sourceLinkIndices.at(j), sourceLinkIndices.at(i)});
            }
        }
    }
}

class BranchStopper
{
    public:
        using Config = std::optional<std::variant<Acts::TrackSelector::Config, Acts::TrackSelector::EtaBinnedConfig>>;
        using BranchStopperResult = Acts::CombinatorialKalmanFilterBranchStopperResult;
        
        mutable std::atomic<std::size_t> m_nStoppedBranches{0};
        explicit BranchStopper(const Config& config) : m_config(config) {}
        
        BranchStopperResult operator()( const Acts::CombinatorialKalmanFilterTipState& tipState,
                                        Acts::VectorMultiTrajectory::TrackStateProxy& trackState ) const
        {
            if (!m_config.has_value()) { return BranchStopperResult::Continue; }

            const Acts::TrackSelector::Config* singleConfig = std::visit
            (
                [&](const auto& config) -> const Acts::TrackSelector::Config*
                {
                    using T = std::decay_t<decltype(config)>;
                    if constexpr (std::is_same_v<T, Acts::TrackSelector::Config>) { return &config; }
                    else if constexpr (std::is_same_v<T, Acts::TrackSelector::EtaBinnedConfig>)
                    {
                        double theta = trackState.parameters()[Acts::eBoundTheta];
                        double eta = -std::log(std::tan(0.5 * theta));
                        return config.hasCuts(eta) ? &config.getCuts(eta) : nullptr;
                    }
                }, *m_config
            );
            
            if (singleConfig == nullptr)
            {
                ++m_nStoppedBranches;
                return BranchStopperResult::StopAndDrop;
            }

            bool enoughMeasurements =
            tipState.nMeasurements >= singleConfig->minMeasurements;
            bool tooManyHoles = tipState.nHoles > singleConfig->maxHoles;
            bool tooManyOutliers = tipState.nOutliers > singleConfig->maxOutliers;

            if (tooManyHoles || tooManyOutliers) {
                ++m_nStoppedBranches;
                return enoughMeasurements ? BranchStopperResult::StopAndKeep
                : BranchStopperResult::StopAndDrop;
            }
            
            return BranchStopperResult::Continue;
        }

    private:
    Config m_config;
};