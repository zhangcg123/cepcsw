// STL
#include <cmath>
#include <vector>
#include <cstdint>

// boost
#include <boost/container/static_vector.hpp>

// acts tools
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Definitions/Common.hpp"
#include "Acts/EventData/SourceLink.hpp"
#include "Acts/Seeding/Seed.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"

// edm4hep
#include "edm4hep/TrackerHit.h"
#include "edm4hep/SimTrackerHit.h"

// local
#include "GeometryContainers.hpp"

using Index = std::uint32_t;
template <typename value_t>
using IndexMultimap = boost::container::flat_multimap<Index, value_t>;

template <typename value_t>
inline boost::container::flat_multimap<value_t, Index> invertIndexMultimap(
    const IndexMultimap<value_t>& multimap) {
  using InverseMultimap = boost::container::flat_multimap<value_t, Index>;

  // switch key-value without enforcing the new ordering (linear copy)
  typename InverseMultimap::sequence_type unordered;
  unordered.reserve(multimap.size());
  for (auto&& [index, value] : multimap) {
    // value is now the key and the index is now the value
    unordered.emplace_back(value, index);
  }

  // adopting the unordered sequence will reestablish the correct order
  InverseMultimap inverse;
#if BOOST_VERSION < 107800
  for (const auto& i : unordered) {
    inverse.insert(i);
  }
#else
  inverse.insert(unordered.begin(), unordered.end());
#endif
  return inverse;
}

/// Space point representation of a measurement suitable for track seeding.
class SimSpacePoint {
    using Scalar = Acts::ActsScalar;

public:
    /// Construct the space point from edm4hep::TrackerHit
    ///
    /// @param trackerhit tracker hit to construct the space point from
    SimSpacePoint(const edm4hep::TrackerHit trackerhit,
                  float x, float y, float z, float t,
                //   const edm4hep::SimTrackerHit simtrackerhit,
                //   Acts::GeometryIdentifier geometryId,
                  boost::container::static_vector<Acts::SourceLink, 2> sourceLinks)
                  : m_x(x), m_y(y), m_z(z), m_t(t),
                  m_sourceLinks(std::move(sourceLinks))
                //   , m_geometryId(geometryId)
    {
        // m_trackerHit = trackerhit;
        // m_simtrackerHit = simtrackerhit;
        // m_x = simtrackerhit.getPosition()[0];
        // m_y = simtrackerhit.getPosition()[1];
        // m_z = simtrackerhit.getPosition()[2];
        // m_t = simtrackerhit.getTime();
        // m_cellid = simtrackerhit.getCellID();
        m_rho = std::sqrt(m_x * m_x + m_y * m_y + m_z * m_z);
        m_varianceRho = trackerhit.getCovMatrix()[0];
        m_varianceZ = trackerhit.getCovMatrix()[5];
    }

    // edm4hep::TrackerHit getTrackerHit() { return m_trackerHit; }
    // edm4hep::SimTrackerHit getSimTrackerHit() { return m_simtrackerHit; }

    constexpr Scalar x() const { return m_x; }
    constexpr Scalar y() const { return m_y; }
    constexpr Scalar z() const { return m_z; }
    constexpr std::optional<Scalar> t() const { return m_t; }
    constexpr Scalar r() const { return m_rho; }
    constexpr Scalar varianceR() const { return m_varianceRho; }
    constexpr Scalar varianceZ() const { return m_varianceZ; }
    constexpr std::optional<Scalar> varianceT() const { return m_varianceT; }

    // constexpr std::uint64_t cellid() const { return m_cellid; }
    // constexpr Acts::GeometryIdentifier geometryId() const { return m_geometryId; }

    const boost::container::static_vector<Acts::SourceLink, 2>&
        sourceLinks() const { return m_sourceLinks; }

    constexpr float topHalfStripLength() const { return m_topHalfStripLength; }
    constexpr float bottomHalfStripLength() const { return m_bottomHalfStripLength; }

    Acts::Vector3 topStripDirection() const { return m_topStripDirection; }
    Acts::Vector3 bottomStripDirection() const { return m_bottomStripDirection; }
    Acts::Vector3 stripCenterDistance() const { return m_stripCenterDistance; }
    Acts::Vector3 topStripCenterPosition() const { return m_topStripCenterPosition; }

    constexpr bool validDoubleMeasurementDetails() const {
        return m_validDoubleMeasurementDetails;
    }

private:

    // edm4hep::TrackerHit m_trackerHit;
    // edm4hep::SimTrackerHit m_simtrackerHit;
    // std::uint64_t m_cellid;
    // Acts::GeometryIdentifier m_geometryId;

    // Global position
    Scalar m_x;
    Scalar m_y;
    Scalar m_z;
    std::optional<Scalar> m_t;
    Scalar m_rho;
    // Variance in rho/z of the global coordinates
    Scalar m_varianceRho;
    Scalar m_varianceZ;
    std::optional<Scalar> m_varianceT;
    // SourceLinks of the corresponding measurements. A Pixel (strip) SP has one
    // (two) sourceLink(s).
    boost::container::static_vector<Acts::SourceLink, 2> m_sourceLinks;

    // half of the length of the top strip
    float m_topHalfStripLength = 0;
    // half of the length of the bottom strip
    float m_bottomHalfStripLength = 0;
    // direction of the top strip
    Acts::Vector3 m_topStripDirection = {0, 0, 0};
    // direction of the bottom strip
    Acts::Vector3 m_bottomStripDirection = {0, 0, 0};
    // distance between the center of the two strips
    Acts::Vector3 m_stripCenterDistance = {0, 0, 0};
    // position of the center of the bottom strip
    Acts::Vector3 m_topStripCenterPosition = {0, 0, 0};
    bool m_validDoubleMeasurementDetails = false;

}; // SimSpacePoint

inline bool operator==(const SimSpacePoint& lhs, const SimSpacePoint& rhs) {
    // (TODO) use sourceLinks for comparison
    return ((lhs.x() == rhs.x()) && (lhs.y() == rhs.y()) &&
            (lhs.z() == rhs.z()) && (lhs.t() == rhs.t()));
}

/// Container of space points.
using SimSpacePointContainer = std::vector<SimSpacePoint>;
using SimSeed = Acts::Seed<SimSpacePoint>;
using SimSeedContainer = std::vector<Acts::Seed<SimSpacePoint>>;


// --------------------------
// IndexSourceLink
// --------------------------

/// A source link that stores just an index.
///
/// This is intentionally kept as barebones as possible. The source link
/// is just a reference and will be copied, moved around, etc. often.
/// Keeping it small and separate from the actual, potentially large,
/// measurement data should result in better overall performance.
/// Using an index instead of e.g. a pointer, means source link and
/// measurement are decoupled and the measurement representation can be
/// easily changed without having to also change the source link.
class IndexSourceLink final
{
    public:
        /// Construct from geometry identifier and index.
        IndexSourceLink(Acts::GeometryIdentifier gid, Index idx, edm4hep::TrackerHit trackhit)
            : m_geometryId(gid), m_index(idx), m_trackhit(trackhit) {}

        // Construct an invalid source link.
        // Must be default constructible to satisfy SourceLinkConcept.
        IndexSourceLink() = default;
        IndexSourceLink(const IndexSourceLink&) = default;
        IndexSourceLink(IndexSourceLink&&) = default;
        IndexSourceLink& operator=(const IndexSourceLink&) = default;
        IndexSourceLink& operator=(IndexSourceLink&&) = default;

        /// Access the index.
        Index index() const { return m_index; }
        Acts::GeometryIdentifier geometryId() const { return m_geometryId; }
        edm4hep::TrackerHit getTrackerHit() const { return m_trackhit; }

        struct SurfaceAccessor
        {
            const Acts::TrackingGeometry& trackingGeometry;
            const Acts::Surface* operator()(const Acts::SourceLink& sourceLink) const
            {
                const auto& indexSourceLink = sourceLink.get<IndexSourceLink>();
                return trackingGeometry.findSurface(indexSourceLink.geometryId());
            }
        };

    private:
        Acts::GeometryIdentifier m_geometryId;
        Index m_index = 0;
        edm4hep::TrackerHit m_trackhit;

        friend bool operator==(const IndexSourceLink& lhs, const IndexSourceLink& rhs)
        {
            return (lhs.geometryId() == rhs.geometryId()) && (lhs.m_index == rhs.m_index);
        }

        friend bool operator!=(const IndexSourceLink& lhs, const IndexSourceLink& rhs)
        {
            return !(lhs == rhs);
        }
}; // IndexSourceLink

/// Container of index source links.
///
/// Since the source links provide a `.geometryId()` accessor,
/// they can be stored in an ordered geometry container.
using IndexSourceLinkContainer = GeometryIdMultiset<IndexSourceLink>;

/// Accessor for the above source link container
///
/// It wraps up a few lookup methods to be used in the Combinatorial Kalman
/// Filter
struct IndexSourceLinkAccessor : GeometryIdMultisetAccessor<IndexSourceLink>
{
    using BaseIterator = GeometryIdMultisetAccessor<IndexSourceLink>::Iterator;

    using Iterator = Acts::SourceLinkAdapterIterator<BaseIterator>;

    // get the range of elements with requested geoId
    std::pair<Iterator, Iterator> range(const Acts::Surface& surface) const
    {
        assert(container != nullptr);
        auto [begin, end] = container->equal_range(surface.geometryId());
        return {Iterator{begin}, Iterator{end}};
    }
};