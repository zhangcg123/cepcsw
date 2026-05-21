// STL
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <utility>
#include <iterator>

// boost
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>

// acts
#include "Acts/EventData/SourceLink.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Surfaces/Surface.hpp"

template <typename Iterator>
class Range
{
    public:
        Range(Iterator b, Iterator e) : m_begin(b), m_end(e) {}
        Range(Range&&) = default;
        Range(const Range&) = default;
        ~Range() = default;
        Range& operator=(Range&&) = default;
        Range& operator=(const Range&) = default;

        Iterator begin() const { return m_begin; }
        Iterator end() const { return m_end; }
        bool empty() const { return m_begin == m_end; }
        std::size_t size() const { return std::distance(m_begin, m_end); }

    private:
        Iterator m_begin;
        Iterator m_end;
};

template <typename Iterator>
Range<Iterator> makeRange(Iterator begin, Iterator end)
{ return Range<Iterator>(begin, end); }

template <typename Iterator>
Range<Iterator> makeRange(std::pair<Iterator, Iterator> range)
{ return Range<Iterator>(range.first, range.second); }


/// Proxy for iterating over groups of elements within a container.
///
/// @note Each group will contain at least one element.
///
/// Consecutive elements with the same key (as defined by the KeyGetter) are
/// placed in one group. The proxy should always be used as part of a
/// range-based for loop. In combination with structured bindings to reduce the
/// boilerplate, the group iteration can be written as
///
///     for (auto&& [key, elements] : GroupBy<...>(...)) {
///         // do something with just the key
///         ...
///
///         // iterate over the group elements
///         for (const auto& element : elements) {
///             ...
///         }
///     }
///
template <typename Iterator, typename KeyGetter>
class GroupBy {
 public:
  /// The key type that identifies elements within a group.
  using Key = std::decay_t<decltype(KeyGetter()(*Iterator()))>;
  /// A Group is an iterator range with the associated key.
  using Group = std::pair<Key, Range<Iterator>>;
  /// Iterator type representing the end of the groups.
  ///
  /// The end iterator will not be dereferenced in C++17 range-based loops. It
  /// can thus be a simpler type without the overhead of the full group iterator
  /// below.
  using GroupEndIterator = Iterator;
  /// Iterator type representing a group of elements.
  class GroupIterator {
   public:
    using iterator_category = std::input_iterator_tag;
    using value_type = Group;
    using difference_type = std::ptrdiff_t;
    using pointer = Group*;
    using reference = Group&;

    constexpr GroupIterator(const GroupBy& groupBy, Iterator groupBegin)
        : m_groupBy(groupBy),
          m_groupBegin(groupBegin),
          m_groupEnd(groupBy.findEndOfGroup(groupBegin)) {}
    /// Pre-increment operator to advance to the next group.
    constexpr GroupIterator& operator++() {
      // make the current end the new group beginning
      std::swap(m_groupBegin, m_groupEnd);
      // find the end of the next group starting from the new beginning
      m_groupEnd = m_groupBy.findEndOfGroup(m_groupBegin);
      return *this;
    }
    /// Post-increment operator to advance to the next group.
    constexpr GroupIterator operator++(int) {
      GroupIterator retval = *this;
      ++(*this);
      return retval;
    }
    /// Dereference operator that returns the pointed-to group of elements.
    constexpr Group operator*() const {
      const Key key = (m_groupBegin != m_groupEnd)
                          ? m_groupBy.m_keyGetter(*m_groupBegin)
                          : Key();
      return {key, makeRange(m_groupBegin, m_groupEnd)};
    }

   private:
    const GroupBy& m_groupBy;
    Iterator m_groupBegin;
    Iterator m_groupEnd;

    friend constexpr bool operator==(const GroupIterator& lhs,
                                     const GroupEndIterator& rhs) {
      return lhs.m_groupBegin == rhs;
    }
    friend constexpr bool operator!=(const GroupIterator& lhs,
                                     const GroupEndIterator& rhs) {
      return !(lhs == rhs);
    }
  };

  /// Construct the group-by proxy for an iterator range.
  constexpr GroupBy(Iterator begin, Iterator end,
                    KeyGetter keyGetter = KeyGetter())
      : m_begin(begin), m_end(end), m_keyGetter(std::move(keyGetter)) {}
  constexpr GroupIterator begin() const {
    return GroupIterator(*this, m_begin);
  }
  constexpr GroupEndIterator end() const { return m_end; }
  constexpr bool empty() const { return m_begin == m_end; }

 private:
  Iterator m_begin;
  Iterator m_end;
  KeyGetter m_keyGetter;

  /// Find the end of the group that starts at the given position.
  ///
  /// This uses a linear search from the start position and thus has linear
  /// complexity in the group size. It does not assume any ordering of the
  /// underlying container and is a cache-friendly access pattern.
  constexpr Iterator findEndOfGroup(Iterator start) const {
    // check for end so we can safely dereference the start iterator.
    if (start == m_end) {
      return start;
    }
    // search the first element that does not share a key with the start.
    return std::find_if_not(std::next(start), m_end,
                            [this, start](const auto& x) {
                              return m_keyGetter(x) == m_keyGetter(*start);
                            });
  }
};

/// Construct the group-by proxy for a container.
template <typename Container, typename KeyGetter>
auto makeGroupBy(const Container& container, KeyGetter keyGetter)
    -> GroupBy<decltype(std::begin(container)), KeyGetter> {
  return {std::begin(container), std::end(container), std::move(keyGetter)};
}

// extract the geometry identifier from a variety of types
struct GeometryIdGetter
{
    // explicit geometry identifier are just forwarded
    constexpr Acts::GeometryIdentifier operator()(
        Acts::GeometryIdentifier geometryId) const { return geometryId; }

    // encoded geometry ids are converted back to geometry identifiers.
    constexpr Acts::GeometryIdentifier operator()(
        Acts::GeometryIdentifier::Value encoded) const { return Acts::GeometryIdentifier(encoded); }

    // support elements in map-like structures.
    template <typename T>
    constexpr Acts::GeometryIdentifier operator()(
        const std::pair<Acts::GeometryIdentifier, T>& mapItem) const { return mapItem.first; }

    // support elements that implement `.geometryId()`.
    template <typename T>
    inline auto operator()(const T& thing) const -> 
        decltype(thing.geometryId(), Acts::GeometryIdentifier()) { return thing.geometryId(); }

    // support reference_wrappers around such types as well
    template <typename T>
    inline auto operator()(std::reference_wrapper<T> thing) const ->
        decltype(thing.get().geometryId(), Acts::GeometryIdentifier()) { return thing.get().geometryId(); }
};

struct CompareGeometryId
{
    // indicate that comparisons between keys and full objects are allowed.
    using is_transparent = void;
    // compare two elements using the automatic key extraction.
    template <typename Left, typename Right>
    constexpr bool operator()(Left&& lhs, Right&& rhs) const
    { return GeometryIdGetter()(lhs) < GeometryIdGetter()(rhs); }
};

/// Store elements that know their detector geometry id, e.g. simulation hits.
///
/// @tparam T type to be stored, must be compatible with `CompareGeometryId`
///
/// The container stores an arbitrary number of elements for any geometry
/// id. Elements can be retrieved via the geometry id; elements can be selected
/// for a specific geometry id or for a larger range, e.g. a volume or a layer
/// within the geometry hierarchy using the helper functions below. Elements can
/// also be accessed by index that uniquely identifies each element regardless
/// of geometry id.
template <typename T>
using GeometryIdMultiset = boost::container::flat_multiset<T, CompareGeometryId>;

/// Store elements indexed by an geometry id.
///
/// @tparam T type to be stored
///
/// The behaviour is the same as for the `GeometryIdMultiset` except that the
/// stored elements do not know their geometry id themself. When iterating
/// the iterator elements behave as for the `std::map`, i.e.
///
///     for (const auto& entry: elements) {
///         auto id = entry.first; // geometry id
///         const auto& el = entry.second; // stored element
///     }
///
template <typename T>
using GeometryIdMultimap = GeometryIdMultiset<std::pair<Acts::GeometryIdentifier, T>>;

/// Select all elements within the given volume.
template <typename T>
inline Range<typename GeometryIdMultiset<T>::const_iterator> selectVolume(
    const GeometryIdMultiset<T>& container, Acts::GeometryIdentifier::Value volume)
{
    auto cmp = Acts::GeometryIdentifier().setVolume(volume);
    auto beg = std::lower_bound(container.begin(), container.end(), cmp, CompareGeometryId{});
    // WARNING overflows to volume==0 if the input volume is the last one
    cmp = Acts::GeometryIdentifier().setVolume(volume + 1u);
    // optimize search by using the lower bound as start point. also handles
    // volume overflows since the geo id would be located before the start of
    // the upper edge search window.
    auto end = std::lower_bound(beg, container.end(), cmp, CompareGeometryId{});
    return makeRange(beg, end);
}

/// Select all elements within the given volume.
template <typename T>
inline auto selectVolume(const GeometryIdMultiset<T>& container, Acts::GeometryIdentifier id)
{ 
    return selectVolume(container, id.volume());
}

/// Select all elements within the given layer.
template <typename T>
inline Range<typename GeometryIdMultiset<T>::const_iterator> selectLayer(
    const GeometryIdMultiset<T>& container,
    Acts::GeometryIdentifier::Value volume,
    Acts::GeometryIdentifier::Value layer)
{
    auto cmp = Acts::GeometryIdentifier().setVolume(volume).setLayer(layer);
    auto beg = std::lower_bound(container.begin(), container.end(), cmp, CompareGeometryId{});
    // WARNING resets to layer==0 if the input layer is the last one
    cmp = Acts::GeometryIdentifier().setVolume(volume).setLayer(layer + 1u);
    // optimize search by using the lower bound as start point. also handles
    // volume overflows since the geo id would be located before the start of
    // the upper edge search window.
    auto end = std::lower_bound(beg, container.end(), cmp, CompareGeometryId{});
    return makeRange(beg, end);
}

// Select all elements within the given layer.
template <typename T>
inline auto selectLayer(const GeometryIdMultiset<T>& container, Acts::GeometryIdentifier id)
{
    return selectLayer(container, id.volume(), id.layer());
}

/// Select all elements for the given module / sensitive surface.
template <typename T>
inline Range<typename GeometryIdMultiset<T>::const_iterator> selectModule(
const GeometryIdMultiset<T>& container, Acts::GeometryIdentifier geoId)
{
    // module is the lowest level and defines a single geometry id value
    return makeRange(container.equal_range(geoId));
}

/// Select all elements for the given module / sensitive surface.
template <typename T>
inline auto selectModule(
    const GeometryIdMultiset<T>& container,
    Acts::GeometryIdentifier::Value volume,
    Acts::GeometryIdentifier::Value layer,
    Acts::GeometryIdentifier::Value module)
{
    return selectModule(container,
                        Acts::GeometryIdentifier().setVolume(volume).setLayer(layer).setSensitive(module));
}

/// Select all elements for the lowest non-zero identifier component.
///
/// Zero values of lower components are interpreted as wildcard search patterns
/// that select all element at the given geometry hierarchy and below. This only
/// applies to the lower components and not to intermediate zeros.
///
/// Examples:
/// - volume=2,layer=0,module=3 -> select all elements in the module
/// - volume=1,layer=2,module=0 -> select all elements in the layer
/// - volume=3,layer=0,module=0 -> select all elements in the volume
///
/// @note An identifier with all components set to zero selects the whole input
///   container.
/// @note Boundary and approach surfaces do not really fit into the geometry
///   hierarchy and must be set to zero for the selection. If they are set on an
///   input identifier, the behaviour of this search method is undefined.
template <typename T>
inline Range<typename GeometryIdMultiset<T>::const_iterator>
selectLowestNonZeroGeometryObject(const GeometryIdMultiset<T>& container, Acts::GeometryIdentifier geoId)
{
    assert((geoId.boundary() == 0u) && "Boundary component must be zero");
    assert((geoId.approach() == 0u) && "Approach component must be zero");

    if (geoId.sensitive() != 0u) { return selectModule(container, geoId); }
    else if (geoId.layer() != 0u) { return selectLayer(container, geoId); }
    else if (geoId.volume() != 0u) { return selectVolume(container, geoId); }
    else { return makeRange(container.begin(), container.end()); }
}

/// Iterate over groups of elements belonging to each module/ sensitive surface.
template <typename T>
inline GroupBy<typename GeometryIdMultiset<T>::const_iterator, GeometryIdGetter>
groupByModule(const GeometryIdMultiset<T>& container)
{
    return makeGroupBy(container, GeometryIdGetter());
}

/// The accessor for the GeometryIdMultiset container
///
/// It wraps up a few lookup methods to be used in the Combinatorial Kalman
/// Filter
template <typename T>
struct GeometryIdMultisetAccessor {
using Container = GeometryIdMultiset<T>;
using Key = Acts::GeometryIdentifier;
using Value = typename GeometryIdMultiset<T>::value_type;
using Iterator = typename GeometryIdMultiset<T>::const_iterator;

// pointer to the container
const Container* container = nullptr;
};