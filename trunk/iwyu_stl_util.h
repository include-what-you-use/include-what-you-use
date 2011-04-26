//===--- iwyu_stl_util.h - STL-like utilities for include-what-you-use ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Utilities that make it easier to work with STL.

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_STL_UTIL_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_STL_UTIL_H_

#include <algorithm>  // for find
#include <map>
#include <set>
#include <utility>  // for pair<>
#include <vector>
#include "port.h"

namespace include_what_you_use {

using std::map;
using std::multimap;
using std::pair;
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
// returns NULL.
template <typename K, typename V>
const V* FindInMap(const map<K, V>* a_map, const K& key) {
  const typename map<K, V>::const_iterator it = a_map->find(key);
  return it == a_map->end() ? NULL : &it->second;
}
template <typename K, typename V>
V* FindInMap(map<K, V>* a_map, const K& key) {
  const typename map<K, V>::iterator it = a_map->find(key);
  return it == a_map->end() ? NULL : &it->second;
}

// Returns all values associated with the given key in the multimap.
template <typename K, typename V>
vector<V> FindInMultiMap(const multimap<K, V>& a_multimap, const K& key) {
  vector<V> retval;
  for (typename multimap<K, V>::const_iterator it = a_multimap.lower_bound(key),
           end = a_multimap.upper_bound(key); it != end; ++it) {
    retval.push_back(it->second);
  }
  return retval;
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


// Utilities for writing concise loops over STL containers.
//
//   for (Each<T> it(&some_container); !it.AtEnd(); ++it) {
//     ... access the current element via *it or it->something ...
//   }
//
//   for (Each<Key, Value> it(&some_map); !it.AtEnd(); ++it) {
//     ... access the key via it->first ...
//     ... access the value via it->second ...
//   }
//
// Benefit of Each over concise_iterator.h:
//
//   - Only the element type (as opposed to the entire container type)
//     needs to be specified.
//   - Safer as it doesn't allow the container to be a temporary object.
//
// Disadvantage:
//
//   - Slower (AtEnd(), ++, and iterator dereference all involve a
//     virtual call).
template <typename T, typename U = void>
class Each;

template <typename Element>
class Each<Element, void> {  // implements Each<Element>
 public:
  template <class Container>
  explicit Each(const Container* container)
      : impl_(new Impl<typename Container::const_iterator>(container->begin(),
                                                           container->end())) {}
  ~Each() { delete impl_; }

  // Returns true if the iterator points to the end of the container.
  bool AtEnd() const { return impl_->AtEnd(); }

  // Advances the iterator.
  void operator++() { impl_->Advance(); }

  // Reads the current element.
  const Element& operator*() const { return *impl_->Get(); }
  const Element* operator->() const { return impl_->Get(); }

 private:
  class ImplBase {
   public:
    virtual ~ImplBase() {}

    virtual bool AtEnd() const = 0;
    virtual void Advance() = 0;
    virtual const Element* Get() const = 0;
  };

  template <typename Iter>
  class Impl : public ImplBase {
   public:
    Impl(Iter begin, Iter end)
        : current_(begin),
          end_(end) {}

    virtual bool AtEnd() const { return current_ == end_; }
    virtual void Advance() { ++current_; }
    virtual const Element* Get() const { return &(*current_); }

   private:
    Iter current_;
    const Iter end_;
  };

  Each(const Each&);  // No implementation.
  void operator=(const Each&);  // No implementation.

  ImplBase* const impl_;
};

// Each<Key, Value> is just a short-hand for Each<pair<const Key, Value> >.
template <typename Key, typename Value>
class Each : public Each<pair<const Key, Value> > {
 public:
  template <class Container>
  explicit Each(const Container* container)
      : Each<pair<const Key, Value> >(container) {}
};

}  // namespace include_what_you_use

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_STL_UTIL_H_
