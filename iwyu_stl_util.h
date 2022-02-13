//===--- iwyu_stl_util.h - STL-like utilities for include-what-you-use ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Utilities that make it easier to work with STL.

#ifndef INCLUDE_WHAT_YOU_USE_IWYU_STL_UTIL_H_
#define INCLUDE_WHAT_YOU_USE_IWYU_STL_UTIL_H_

#include <algorithm>                    // for find
#include <map>                          // for map, multimap
#include <set>                          // for set
#include <vector>                       // for vector

namespace include_what_you_use {

using std::map;
using std::set;
using std::vector;

// Returns true if the associative container (e.g. set or map)
// contains the given key.
template <class AssociativeContainer>
bool ContainsKey(const AssociativeContainer& container,
                 const typename AssociativeContainer::key_type& key) {
  return container.find(key) != container.end();
}

// Returns true if the container contains the given value.
template <class Container>
bool ContainsValue(const Container& container,
                   const typename Container::value_type& value) {
  return (std::find(container.begin(), container.end(), value)
          != container.end());
}

// For maps, we also let you check if the key exists with the given value.
template <class Container, typename K, typename V>
bool ContainsKeyValue(const Container& container,
                      const K& key, const V& value) {
  for (typename Container::const_iterator it = container.lower_bound(key),
           end = container.upper_bound(key); it != end; ++it) {
    if (it->second == value)
      return true;
  }
  return false;
}

// Returns true if the associative container contains any key in the
// given set.
template <class AssociativeContainer>
bool ContainsAnyKey(
    const AssociativeContainer& container,
    const set<typename AssociativeContainer::key_type>& keys) {
  for (const auto& key : keys) {
    if (ContainsKey(container, key))
      return true;
  }
  return false;
}

// Returns a_map[key] if key is in a_map; otherwise returns default_value.
template <class Map>
const typename Map::mapped_type& GetOrDefault(
    const Map& a_map, const typename Map::key_type& key,
    const typename Map::mapped_type& default_value) {
  if (ContainsKey(a_map, key))
    return a_map.find(key)->second;
  return default_value;
}

// Returns a pointer to (*a_map)[key] if key is in *a_map; otherwise
// returns nullptr.
template <typename K, typename V>
const V* FindInMap(const map<K, V>* a_map, const K& key) {
  const typename map<K, V>::const_iterator it = a_map->find(key);
  return it == a_map->end() ? nullptr : &it->second;
}
template <typename K, typename V>
V* FindInMap(map<K, V>* a_map, const K& key) {
  const typename map<K, V>::iterator it = a_map->find(key);
  return it == a_map->end() ? nullptr : &it->second;
}

// Removes all elements in source from target.
template <class SourceContainer, class TargetContainer>
void RemoveAllFrom(const SourceContainer& source, TargetContainer* target) {
  for (typename SourceContainer::const_iterator it = source.begin();
       it != source.end(); ++it) {
    target->erase(*it);
  }
}

// Inserts all elements from source into target.
template <class SourceContainer, class TargetContainer>
void InsertAllInto(const SourceContainer& source, TargetContainer* target) {
  target->insert(source.begin(), source.end());
}

// Appends all elements from source to the end of target.  The target
// type must support inserting a range at the end, which probably
// means it's a vector.
template <class TargetContainer, class SourceContainer>
void Extend(TargetContainer* target, const SourceContainer& source) {
  target->insert(target->end(), source.begin(), source.end());
}

// Returns the union of the two given sets.
template <typename T>
set<T> Union(const set<T>& lhs, const set<T>& rhs) {
  set<T> retval(lhs);
  InsertAllInto(rhs, &retval);
  return retval;
}

// Returns a vector v with all duplicates removed, but order otherwise
// maintained.
template <typename T>
vector<T> GetUniqueEntries(const vector<T>& v) {
  set<T> seen;
  vector<T> retval;
  for (typename vector<T>::const_iterator it = v.begin(); it != v.end(); ++it) {
    if (!ContainsKey(seen, *it)) {
      retval.push_back(*it);
      seen.insert(*it);
    }
  }
  return retval;
}

}  // namespace include_what_you_use

#endif  // INCLUDE_WHAT_YOU_USE_IWYU_STL_UTIL_H_
