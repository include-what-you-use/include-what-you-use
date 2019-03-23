//===--- iwyu_include_picker.cc - map to canonical #includes for iwyu -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_include_picker.h"

#include <algorithm>                    // for find
#include <cstddef>                      // for size_t
// TODO(wan): make sure IWYU doesn't suggest <iterator>.
#include <iterator>                     // for find
// not hash_map: it's not as portable and needs hash<string>.
#include <map>                          // for map, map<>::mapped_type, etc
#include <memory>
#include <ostream>
#include <string>                       // for string, basic_string, etc
#include <system_error>                 // for error_code
#include <utility>                      // for pair, make_pair
#include <vector>                       // for vector, vector<>::iterator

#include "iwyu_location_util.h"
#include "iwyu_path_util.h"
#include "iwyu_stl_util.h"
#include "iwyu_string_util.h"
#include "iwyu_verrs.h"
#include "port.h"  // for CHECK_

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Regex.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLParser.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/Basic/FileManager.h"

using std::find;
using std::make_pair;
using std::map;
using std::pair;
using std::string;
using std::unique_ptr;
using std::vector;

using llvm::MemoryBuffer;
using llvm::SourceMgr;
using llvm::errs;
using llvm::yaml::KeyValueNode;
using llvm::yaml::MappingNode;
using llvm::yaml::Node;
using llvm::yaml::ScalarNode;
using llvm::yaml::SequenceNode;
using llvm::yaml::Stream;
using llvm::yaml::document_iterator;

namespace include_what_you_use {

// If we map from A to B, it means that every time we need a
// symbol from A, we can also get it from B.  Another way
// to think about it is that map_to "re-exports" all the
// symbols from map_from.
struct IncludeMapEntry {      // A POD so we can make the input static
  const char* map_from;       // A quoted-include or a symbol name
  IncludeVisibility from_visibility;
  const char* map_to;         // A quoted-include
  IncludeVisibility to_visibility;
};

namespace {

// Listed below are all IWYU's native symbol and include mappings,
// loosely based on GCC 4.4's libc and libstdc++.
#include "gcc_libc_symbol_map.h"
#include "gcc_libc_include_map.h"
#include "stl_c_headers_include_map.h"
#include "gcc_stl_headers_include_map.h"
#include "libstdcpp_symbol_map.h"

// For comparing if maps are equivelent.
static bool CompareIncludeMaps(const IncludeMapEntry* A,
                               size_t ACount,
                               const IncludeMapEntry* B,
                               size_t BCount) {
  bool ret = true;
  if (ACount != BCount) {
    fprintf(stderr, "Error : map sizes do not match %zu vs %zu\n",
	    ACount, BCount);
    ret = false;
  }
  size_t c = ACount < BCount ? ACount : BCount;
  for (size_t i = 0; i < c; i++) {
    //printf("%d from %s %s\n", i, A[i].map_from, B[i].map_from);
    //printf("%d to   %s %s\n", i, A[i].map_to, B[i].map_to);
    if (!((0 == strcmp(A[i].map_from, B[i].map_from)) &&
          (0 == strcmp(B[i].map_from, A[i].map_from)))) {
      fprintf(stderr, "Error : map_from for entry %zu do not match %s vs %s\n",
              i, A[i].map_from, B[i].map_from);
      ret = false;
      break;
    }
    if (A[i].from_visibility != B[i].from_visibility) {
      fprintf(stderr, "Error : from_visibility for entry %zu do not match %u vs %u\n",
              i, A[i].from_visibility, B[i].from_visibility);
      ret = false;
      break;
    }
    if (!((0 == strcmp(A[i].map_to, B[i].map_to)) &&
          (0 == strcmp(B[i].map_to, A[i].map_to)))) {
      fprintf(stderr, "Error : map_to for entry %zu do not match %s vs %s\n",
              i, A[i].map_to, B[i].map_to);
      ret = false;
      break;
    }
    if (A[i].to_visibility != B[i].to_visibility) {
      fprintf(stderr, "Error : to_visibility for entry %zu do not match %u vs %u\n",
              i, A[i].to_visibility, B[i].to_visibility);
      ret = false;
      break;
    }
  }
  if (ret)
    fprintf(stderr, "PASS\n");
  else
    fprintf(stderr, "FAIL\n");
  return ret;
}

const char* stdlib_cpp_public_headers[] = {
  // These headers are defined in C++14 [headers]p2.  You can get them with
  // $ sed -n '/begin{floattable}.*{tab:cpp.library.headers}/,/end{floattable}/p' lib-intro.tex | grep tcode | perl -nle 'm/tcode{(.*)}/ && print qq@  "$1",@' | sort
  // on https://github.com/cplusplus/draft/blob/master/source/lib-intro.tex
  "<algorithm>",
  "<array>",
  "<atomic>",
  "<bitset>",
  "<chrono>",
  "<codecvt>",
  "<complex>",
  "<condition_variable>",
  "<deque>",
  "<exception>",
  "<forward_list>",
  "<fstream>",
  "<functional>",
  "<future>",
  "<initializer_list>",
  "<iomanip>",
  "<ios>",
  "<iosfwd>",
  "<iostream>",
  "<istream>",
  "<iterator>",
  "<limits>",
  "<list>",
  "<locale>",
  "<map>",
  "<memory>",
  "<mutex>",
  "<new>",
  "<numeric>",
  "<ostream>",
  "<queue>",
  "<random>",
  "<ratio>",
  "<regex>",
  "<scoped_allocator>",
  "<set>",
  "<sstream>",
  "<stack>",
  "<stdexcept>",
  "<streambuf>",
  "<string>",
  "<strstream>",
  "<system_error>",
  "<thread>",
  "<tuple>",
  "<type_traits>",
  "<typeindex>",
  "<typeinfo>",
  "<unordered_map>",
  "<unordered_set>",
  "<utility>",
  "<valarray>",
  "<vector>",
};

// Returns true if str is a valid quoted filepath pattern (i.e. either
// a quoted filepath or "@" followed by a regex for matching a quoted
// filepath).
bool IsQuotedFilepathPattern(const string& str) {
  return IsQuotedInclude(str) || StartsWith(str, "@");
}

// Given a vector of nodes, augment each node with its children, as
// defined by m: nodes[i] is replaced by nodes[i] + m[nodes[i]],
// ignoring duplicates.  The input vector is modified in place.
void ExpandOnce(const IncludePicker::IncludeMap& m, vector<string>* nodes) {
  vector<string> nodes_and_children;
  set<string> seen_nodes_and_children;
  for (const string& node : *nodes) {
    // First insert the node itself, then all its kids.
    if (!ContainsKey(seen_nodes_and_children, node)) {
      nodes_and_children.push_back(node);
      seen_nodes_and_children.insert(node);
    }
    if (const vector<string>* children = FindInMap(&m, node)) {
      for (const string& child : *children) {
        if (!ContainsKey(seen_nodes_and_children, child)) {
          nodes_and_children.push_back(child);
          seen_nodes_and_children.insert(child);
        }
      }
    }
  }
  nodes->swap(nodes_and_children);  // modify nodes in-place
}

enum TransitiveStatus { kUnused = 0, kCalculating, kDone };

// If the filename-map maps a.h to b.h, and also b.h to c.h, then
// there's a transitive mapping of a.h to c.h.  We want to add that
// into the filepath map as well, to make lookups easier.  We do this
// by doing a depth-first search for a single mapping, recursing
// whenever the value is itself a key in the map, and putting the
// results in a vector of all values seen.
// NOTE: This function updates values seen in filename_map, but
// does not invalidate any filename_map iterators.
void MakeNodeTransitive(IncludePicker::IncludeMap* filename_map,
                        map<string, TransitiveStatus>* seen_nodes,
                        vector<string>* node_stack,  // used for debugging
                        const string& key) {
  // If we've already calculated this node's transitive closure, we're done.
  const TransitiveStatus status = (*seen_nodes)[key];
  if (status == kCalculating) {   // means there's a cycle in the mapping
    // TODO: Reconsider cycle handling; the include_cycle test fails without
    // this special-casing, but it seems we should handle this more generally.
    if (key.find("internal/") != string::npos) {
      VERRS(4) << "Ignoring a cyclical mapping involving " << key << "\n";
      return;
    }
  }
  if (status == kCalculating) {
    VERRS(0) << "Cycle in include-mapping:\n";
    for (const string& node : *node_stack)
      VERRS(0) << "  " << node << " ->\n";
    VERRS(0) << "  " << key << "\n";
    CHECK_UNREACHABLE_("Cycle in include-mapping");  // cycle is a fatal error
  }
  if (status == kDone)
    return;

  IncludePicker::IncludeMap::iterator node = filename_map->find(key);
  if (node == filename_map->end()) {
    (*seen_nodes)[key] = kDone;
    return;
  }

  // Keep track of node->second as we update it, to avoid duplicates.
  (*seen_nodes)[key] = kCalculating;
  for (const string& child : node->second) {
    node_stack->push_back(child);
    MakeNodeTransitive(filename_map, seen_nodes, node_stack, child);
    node_stack->pop_back();
  }
  (*seen_nodes)[key] = kDone;

  // Our transitive closure is just the union of the closure of our
  // children.  This routine replaces our value with this closure,
  // by replacing each of our values with its values.  Since our
  // values have already been made transitive, that is a closure.
  ExpandOnce(*filename_map, &node->second);
}

// Updates the values in filename_map based on its transitive mappings.
void MakeMapTransitive(IncludePicker::IncludeMap* filename_map) {
  // Insert keys of filename_map here once we know their value is
  // the complete transitive closure.
  map<string, TransitiveStatus> seen_nodes;
  vector<string> node_stack;
  for (const IncludePicker::IncludeMap::value_type& includes : *filename_map)
    MakeNodeTransitive(filename_map, &seen_nodes, &node_stack, includes.first);
}

// Get a scalar value from a YAML node.
// Returns empty string if it's not of type ScalarNode.
string GetScalarValue(Node* node) {
  ScalarNode* scalar = llvm::dyn_cast<ScalarNode>(node);
  if (scalar == nullptr)
    return string();

  llvm::SmallString<8> storage;
  return scalar->getValue(storage).str();
}

// Get a sequence value from a YAML node.
// Returns empty vector if it's not of type SequenceNode.
vector<string> GetSequenceValue(Node* node) {
  vector<string> result;

  SequenceNode* sequence = llvm::dyn_cast<SequenceNode>(node);
  if (sequence != nullptr) {
    for (Node& node : *sequence) {
      result.push_back(GetScalarValue(&node));
    }
  }

  return result;
}

// If new_path doesn't already exist in search_path, makes a copy of search_path
// and adds new_path to it.
// Returns the original or extended search path.
vector<string> ExtendMappingFileSearchPath(const vector<string>& search_path,
                                           const string& new_path) {
  CHECK_(IsAbsolutePath(new_path));

  if (std::find(search_path.begin(),
                search_path.end(),
                new_path) == search_path.end()) {
    vector<string> extended(search_path);
    extended.push_back(new_path);
    return extended;
  }

  return search_path;
}

// Scans search_path for existing files with filename.
// If filename is absolute and exists, return it.
// If filename is relative and exists based on cwd, return it in absolute form.
// If filename is relative and doesn't exist, try to find it along search_path.
// Returns an absolute filename if file is found, otherwise filename untouched.
string FindFileInSearchPath(const vector<string>& search_path,
                            const string& filename) {
  if (llvm::sys::fs::exists(filename)) {
    // If the file exists, no matter if its path is relative or absolute,
    // return it in absolute form.
    return MakeAbsolutePath(filename);
  } else if (!IsAbsolutePath(filename)) {
    // If it's relative, scan search path.
    for (const string& base_path : search_path) {
      string candidate = MakeAbsolutePath(base_path, filename);
      if (llvm::sys::fs::exists(candidate)) {
        return candidate;
      }
    }
  }

  // This is proven not to exist, so handle the error when
  // we attempt to open it.
  return filename;
}

}  // anonymous namespace

IncludePicker::IncludePicker(bool no_default_mappings)
    : has_called_finalize_added_include_lines_(false) {
  if (!no_default_mappings) {
    AddDefaultMappings();
  }
}

void IncludePicker::AddDefaultMappings() {
  AddSymbolMappings(libc_symbol_map, IWYU_ARRAYSIZE(libc_symbol_map));
  AddSymbolMappings(libstdcpp_symbol_map, IWYU_ARRAYSIZE(libstdcpp_symbol_map));

  AddIncludeMappings(libc_include_map,
      IWYU_ARRAYSIZE(libc_include_map));
  AddIncludeMappings(stdlib_c_include_map,
      IWYU_ARRAYSIZE(stdlib_c_include_map));
  AddIncludeMappings(libstdcpp_include_map,
      IWYU_ARRAYSIZE(libstdcpp_include_map));

  AddPublicIncludes(stdlib_cpp_public_headers,
      IWYU_ARRAYSIZE(stdlib_cpp_public_headers));
}

void IncludePicker::MarkVisibility(const string& quoted_filepath_pattern,
                                   IncludeVisibility visibility) {
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't mutate anymore");

  // insert() leaves any old value alone, and only inserts if the key is new.
  filepath_visibility_map_.insert(
      make_pair(quoted_filepath_pattern, visibility));
  CHECK_(filepath_visibility_map_[quoted_filepath_pattern] == visibility)
      << " Same file seen with two different visibilities: "
      << quoted_filepath_pattern
      << " Old vis: " << filepath_visibility_map_[quoted_filepath_pattern]
      << " New vis: " << visibility;
}

// AddDirectInclude lets us use some hard-coded rules to add filepath
// mappings at runtime.  It includes, for instance, mappings from
// 'project/internal/foo.h' to 'project/public/foo_public.h' in google
// code (Google hides private headers in /internal/, much like glibc
// hides them in /bits/.)
void IncludePicker::AddDirectInclude(const string& includer_filepath,
                                     const string& includee_filepath,
                                     const string& quoted_include_as_written) {
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't mutate anymore");

  // Note: the includer may be a .cc file, which is unnecessary to add
  // to our map, but harmless.
  const string quoted_includer = ConvertToQuotedInclude(includer_filepath);
  const string quoted_includee = ConvertToQuotedInclude(includee_filepath);

  quoted_includes_to_quoted_includers_[quoted_includee].insert(quoted_includer);
  const pair<string, string> key(includer_filepath, includee_filepath);
  includer_and_includee_to_include_as_written_[key] = quoted_include_as_written;

  // Mark the clang fake-file "<built-in>" as private, so we never try
  // to map anything to it.
  if (includer_filepath == "<built-in>")
    MarkIncludeAsPrivate("\"<built-in>\"");

  // Automatically mark files in foo/internal/bar as private, and map them.
  // Then say that everyone else in foo/.* is a friend, who is allowed to
  // include the otherwise-private header.
  const size_t internal_pos = quoted_includee.find("internal/");
  if (internal_pos != string::npos &&
      (internal_pos == 0 || quoted_includee[internal_pos - 1] == '/')) {
    MarkIncludeAsPrivate(quoted_includee);
    // The second argument here is a regex for matching a quoted
    // filepath.  We get the opening quote from quoted_includee, and
    // the closing quote as part of the .*.
    AddFriendRegex(includee_filepath,
                   quoted_includee.substr(0, internal_pos) + ".*");
    AddMapping(quoted_includee, quoted_includer);
  }

  // Automatically mark <asm-FOO/bar.h> as private, and map to <asm/bar.h>.
  if (StartsWith(quoted_includee, "<asm-")) {
    MarkIncludeAsPrivate(quoted_includee);
    string public_header = quoted_includee;
    StripPast(&public_header, "/");   // read past "asm-whatever/"
    public_header = "<asm/" + public_header;   // now it's <asm/something.h>
    AddMapping(quoted_includee, public_header);
  }
}

void IncludePicker::AddMapping(const string& map_from, const string& map_to) {
  VERRS(8) << "Adding mapping from " << map_from << " to " << map_to << "\n";
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't mutate anymore");
  CHECK_(IsQuotedFilepathPattern(map_from)
         && "All map keys must be quoted filepaths or @ followed by regex");
  CHECK_(IsQuotedInclude(map_to) && "All map values must be quoted includes");
  filepath_include_map_[map_from].push_back(map_to);
}

void IncludePicker::AddIncludeMapping(const string& map_from,
                                      IncludeVisibility from_visibility,
                                      const string& map_to,
                                      IncludeVisibility to_visibility) {
  AddMapping(map_from, map_to);
  MarkVisibility(map_from, from_visibility);
  MarkVisibility(map_to, to_visibility);
}

void IncludePicker::AddSymbolMapping(const string& map_from,
                                     const string& map_to,
                                     IncludeVisibility to_visibility) {
  CHECK_(IsQuotedInclude(map_to) && "Map values must be quoted includes");
  symbol_include_map_[map_from].push_back(map_to);

  // Symbol-names are always marked as private (or GetPublicValues()
  // will self-map them, below).
  MarkVisibility(map_from, kPrivate);
  MarkVisibility(map_to, to_visibility);
}

void IncludePicker::AddIncludeMappings(const IncludeMapEntry* entries,
                                       size_t count) {
  for (size_t i = 0; i < count; ++i) {
    const IncludeMapEntry& e = entries[i];
    AddIncludeMapping(e.map_from, e.from_visibility, e.map_to, e.to_visibility);
  }
}

void IncludePicker::AddSymbolMappings(const IncludeMapEntry* entries,
                                      size_t count) {
  for (size_t i = 0; i < count; ++i) {
    const IncludeMapEntry& e = entries[i];
    AddSymbolMapping(e.map_from, e.map_to, e.to_visibility);
  }
}

void IncludePicker::AddPublicIncludes(const char** includes, size_t count) {
  for (size_t i = 0; i < count; ++i) {
    const char* include = includes[i];
    MarkVisibility(include, kPublic);
  }
}

void IncludePicker::MarkIncludeAsPrivate(
    const string& quoted_filepath_pattern) {
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't mutate anymore");
  CHECK_(IsQuotedFilepathPattern(quoted_filepath_pattern)
         && "MIAP takes a quoted filepath pattern");
  MarkVisibility(quoted_filepath_pattern, kPrivate);
}

void IncludePicker::AddFriendRegex(const string& includee_filepath,
                                   const string& quoted_friend_regex) {
  friend_to_headers_map_["@" + quoted_friend_regex].insert(includee_filepath);
}

namespace {

// Given a map keyed by quoted filepath patterns, return a vector
// containing the @-regexes among the keys.
template <typename MapType>
vector<string> ExtractKeysMarkedAsRegexes(const MapType& m) {
  vector<string> regex_keys;
  for (const typename MapType::value_type& item : m) {
    if (StartsWith(item.first, "@"))
      regex_keys.push_back(item.first);
  }
  return regex_keys;
}

}  // anonymous namespace

// Expands the regex keys in filepath_include_map_ and
// friend_to_headers_map_ by matching them against all source files
// seen by iwyu.  For each include that matches the regex, we add it
// to the map by copying the regex entry and replacing the key with
// the seen #include.
void IncludePicker::ExpandRegexes() {
  // First, get the regex keys.
  const vector<string> filepath_include_map_regex_keys =
      ExtractKeysMarkedAsRegexes(filepath_include_map_);
  const vector<string> friend_to_headers_map_regex_keys =
      ExtractKeysMarkedAsRegexes(friend_to_headers_map_);

  // Then, go through all #includes to see if they match the regexes,
  // discarding the identity mappings.  TODO(wan): to improve
  // performance, don't construct more than one Regex object for each
  // element in the above vectors.
  for (const auto& incmap : quoted_includes_to_quoted_includers_) {
    const string& hdr = incmap.first;
    for (const string& regex_key : filepath_include_map_regex_keys) {
      const vector<string>& map_to = filepath_include_map_[regex_key];
      // Enclose the regex in ^(...)$ for full match.
      llvm::Regex regex(std::string("^(" + regex_key.substr(1) + ")$"));
      if (regex.match(hdr, nullptr) && !ContainsValue(map_to, hdr)) {
        Extend(&filepath_include_map_[hdr], filepath_include_map_[regex_key]);
        MarkVisibility(hdr, filepath_visibility_map_[regex_key]);
      }
    }
    for (const string& regex_key : friend_to_headers_map_regex_keys) {
      llvm::Regex regex(std::string("^(" + regex_key.substr(1) + ")$"));
      if (regex.match(hdr, nullptr)) {
        InsertAllInto(friend_to_headers_map_[regex_key],
                      &friend_to_headers_map_[hdr]);
      }
    }
  }
}

// Handle work that's best done after we've seen all the mappings
// (including dynamically-added ones) and all the include files.
// For instance, we can now expand all the regexes we've seen in
// the mapping-keys, since we have the full list of #includes to
// match them again.  We also transitively-close the maps.
void IncludePicker::FinalizeAddedIncludes() {
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't call FAI twice");

  // The map keys may be regular expressions.
  // Match those to seen #includes now.
  ExpandRegexes();

  // If a.h maps to b.h maps to c.h, we'd like an entry from a.h to c.h too.
  MakeMapTransitive(&filepath_include_map_);
  // Now that filepath_include_map_ is transitively closed, it's an
  // easy task to get the values of symbol_include_map_ closed too.
  for (IncludeMap::value_type& symbol_include : symbol_include_map_) {
    ExpandOnce(filepath_include_map_, &symbol_include.second);
  }

  has_called_finalize_added_include_lines_ = true;
}

// For the given key, return the vector of values associated with that
// key, or an empty vector if the key does not exist in the map.
// *However*, we filter out all values that have private visibility
// before returning the vector.  *Also*, if the key is public in
// the map, we insert the key as the first of the returned values,
// this is an implicit "self-map."
vector<string> IncludePicker::GetPublicValues(
    const IncludePicker::IncludeMap& m, const string& key) const {
  CHECK_(!StartsWith(key, "@"));
  vector<string> retval;
  const vector<string>* values = FindInMap(&m, key);
  if (!values || values->empty())
    return retval;

  if (GetOrDefault(filepath_visibility_map_, key, kPublic) == kPublic)
    retval.push_back(key);                // we can map to ourself!
  for (const string& value : *values) {
    CHECK_(!StartsWith(value, "@"));
    if (GetOrDefault(filepath_visibility_map_, value, kPublic) == kPublic)
      retval.push_back(value);
  }
  return retval;
}

string IncludePicker::MaybeGetIncludeNameAsWritten(
    const string& includer_filepath, const string& includee_filepath) const {
  const pair<string, string> key(includer_filepath, includee_filepath);
  // I want to use GetOrDefault here, but it has trouble deducing tpl args.
  const string* value = FindInMap(&includer_and_includee_to_include_as_written_,
                                  key);
  return value ? *value : "";
}

vector<string> IncludePicker::GetCandidateHeadersForSymbol(
    const string& symbol) const {
  CHECK_(has_called_finalize_added_include_lines_ && "Must finalize includes");
  return GetPublicValues(symbol_include_map_, symbol);
}

vector<string> IncludePicker::GetCandidateHeadersForFilepath(
    const string& filepath, const string& including_filepath) const {
  CHECK_(has_called_finalize_added_include_lines_ && "Must finalize includes");
  const string quoted_header = ConvertToQuotedInclude(
      filepath, MakeAbsolutePath(GetParentPath(including_filepath)));
  vector<string> retval = GetPublicValues(filepath_include_map_, quoted_header);
  if (retval.empty()) {
    // the filepath isn't in include_map, so just quote and return it.
    retval.push_back(quoted_header);
  }
  return retval;
}

// Except for the case that the includer is a 'friend' of the includee
// (via an '// IWYU pragma: friend XXX'), the same as
// GetCandidateHeadersForFilepath.
vector<string> IncludePicker::GetCandidateHeadersForFilepathIncludedFrom(
    const string& included_filepath, const string& including_filepath) const {
  vector<string> retval;
  // We pass the own files path to ConvertToQuotedInclude so the quoted include
  // for the case that there is no matching `-I` option is just the filename
  // (e.g. "foo.cpp") instead of the absolute file path.
  const string quoted_includer = ConvertToQuotedInclude(
      including_filepath, MakeAbsolutePath(GetParentPath(including_filepath)));
  const string quoted_includee = ConvertToQuotedInclude(
      included_filepath, MakeAbsolutePath(GetParentPath(including_filepath)));
  const set<string>* headers_with_includer_as_friend =
      FindInMap(&friend_to_headers_map_, quoted_includer);
  if (headers_with_includer_as_friend != nullptr &&
      ContainsKey(*headers_with_includer_as_friend, included_filepath)) {
    retval.push_back(quoted_includee);
  } else {
    retval =
        GetCandidateHeadersForFilepath(included_filepath, including_filepath);
    if (retval.size() == 1) {
      const string& quoted_header = retval[0];
      if (GetVisibility(quoted_header) == kPrivate) {
        VERRS(0) << "Warning: "
                 << "No public header found to replace the private header "
                 << quoted_header << "\n";
      }
    }
  }

  // We'll have called ConvertToQuotedInclude on members of retval,
  // but sometimes we can do better -- if included_filepath is in
  // retval, the iwyu-preprocessor may have stored the quoted-include
  // as written in including_filepath.  This is better to use than
  // ConvertToQuotedInclude because it avoids trouble when the same
  // file is accessible via different include search-paths, or is
  // accessed via a symlink.
  const string& quoted_include_as_written
      = MaybeGetIncludeNameAsWritten(including_filepath, included_filepath);
  if (!quoted_include_as_written.empty()) {
    vector<string>::iterator it = std::find(retval.begin(), retval.end(),
                                            quoted_includee);
    if (it != retval.end())
      *it = quoted_include_as_written;
  }
  return retval;
}

bool IncludePicker::HasMapping(const string& map_from_filepath,
                               const string& map_to_filepath) const {
  CHECK_(has_called_finalize_added_include_lines_ && "Must finalize includes");
  const string quoted_from = ConvertToQuotedInclude(map_from_filepath);
  const string quoted_to = ConvertToQuotedInclude(map_to_filepath);
  // We can't use GetCandidateHeadersForFilepath since includer might be private
  const vector<string>* all_mappers = FindInMap(&filepath_include_map_,
                                                quoted_from);
  if (all_mappers) {
    for (const string& mapper : *all_mappers) {
      if (mapper == quoted_to)
        return true;
    }
  }
  return quoted_to == quoted_from;   // indentity mapping, why not?
}

bool IncludePicker::IsPublic(const clang::FileEntry* file) const {
  CHECK_(file && "Need existing FileEntry");
  const string quoted_file = ConvertToQuotedInclude(GetFilePath(file));
  return (GetVisibility(quoted_file) == kPublic);
}

// Parses a YAML/JSON file containing mapping directives of various types.
void IncludePicker::AddMappingsFromFile(const string& filename) {
  vector<string> default_search_path;
  return AddMappingsFromFile(filename, default_search_path);
}

// Parses a YAML/JSON file containing mapping directives of various types:
//  symbol   - symbol name -> quoted include
//  include  - private quoted include -> public quoted include
//  ref      - include mechanism for mapping files, to allow project-specific
//             groupings
// This private implementation method is recursive and builds the search path
// incrementally.
void IncludePicker::AddMappingsFromFile(const string& filename,
                                        const vector<string>& search_path) {
  string absolute_path = FindFileInSearchPath(search_path, filename);

  llvm::ErrorOr<unique_ptr<MemoryBuffer>> bufferOrError =
      MemoryBuffer::getFile(absolute_path);
  if (std::error_code error = bufferOrError.getError()) {
    errs() << "Cannot open mapping file '" << absolute_path << "': "
           << error.message() << ".\n";
    return;
  }

  VERRS(5) << "Adding mappings from file '" << absolute_path << "'.\n";

  SourceMgr source_manager;
  Stream json_stream(bufferOrError.get()->getMemBufferRef(), source_manager);

  document_iterator stream_begin = json_stream.begin();
  if (stream_begin == json_stream.end())
    return;

  // Get root sequence.
  Node* root = stream_begin->getRoot();
  SequenceNode *array = llvm::dyn_cast<SequenceNode>(root);
  if (array == nullptr) {
    json_stream.printError(root, "Root element must be an array.");
    return;
  }

  for (Node& array_item_node : *array) {
    Node* current_node = &array_item_node;

    // Every item must be a JSON object ("mapping" in YAML terms.)
    MappingNode* mapping = llvm::dyn_cast<MappingNode>(current_node);
    if (mapping == nullptr) {
      json_stream.printError(current_node,
          "Mapping directives must be objects.");
      return;
    }

    for (KeyValueNode &mapping_item_node : *mapping) {
      // General form is { directive: <data> }.
      const string directive = GetScalarValue(mapping_item_node.getKey());

      if (directive == "symbol") {
        // Symbol mapping.
        vector<string> mapping = GetSequenceValue(mapping_item_node.getValue());
        if (mapping.size() != 4) {
          json_stream.printError(current_node,
              "Symbol mapping expects a value on the form "
              "'[from, visibility, to, visibility]'.");
          return;
        }

        // Ignore unused from-visibility, at some point maybe remove it from the
        // mapping file format.

        IncludeVisibility to_visibility = ParseVisibility(mapping[3]);
        if (to_visibility == kUnusedVisibility) {
          json_stream.printError(current_node,
              "Unknown visibility '" + mapping[3] + "'.");
          return;
        }

        if (!IsQuotedInclude(mapping[2])) {
          json_stream.printError(
              current_node,
              "Expected to-entry to be quoted include, but was '" + mapping[2] +
                  "'");
          return;
        }

        AddSymbolMapping(mapping[0], mapping[2], to_visibility);
      } else if (directive == "include") {
        // Include mapping.
        vector<string> mapping = GetSequenceValue(mapping_item_node.getValue());
        if (mapping.size() != 4) {
          json_stream.printError(current_node,
              "Include mapping expects a value on the form "
              "'[from, visibility, to, visibility]'.");
          return;
        }

        IncludeVisibility from_visibility = ParseVisibility(mapping[1]);
        if (from_visibility == kUnusedVisibility) {
          json_stream.printError(current_node,
              "Unknown visibility '" + mapping[1] + "'.");
          return;
        }

        IncludeVisibility to_visibility = ParseVisibility(mapping[3]);
        if (to_visibility == kUnusedVisibility) {
          json_stream.printError(current_node,
              "Unknown visibility '" + mapping[3] + "'.");
          return;
        }

        if (!IsQuotedFilepathPattern(mapping[0])) {
          json_stream.printError(
              current_node,
              "Expected from-entry to be quoted filepath or @regex, but was '" +
                  mapping[0] + "'");
          return;
        }

        if (!IsQuotedInclude(mapping[2])) {
          json_stream.printError(
              current_node,
              "Expected to-entry to be quoted include, but was '" + mapping[2] +
                  "'");
          return;
        }

        AddIncludeMapping(mapping[0],
            from_visibility,
            mapping[2],
            to_visibility);
      } else if (directive == "ref") {
        // Mapping ref.
        string ref_file = GetScalarValue(mapping_item_node.getValue());
        if (ref_file.empty()) {
          json_stream.printError(current_node,
              "Mapping ref expects a single filename value.");
          return;
        }

        // Add the path of the file we're currently processing
        // to the search path. Allows refs to be relative to referrer.
        vector<string> extended_search_path = 
            ExtendMappingFileSearchPath(search_path,
                                        GetParentPath(absolute_path));

        // Recurse.
        AddMappingsFromFile(ref_file, extended_search_path);
      } else {
        json_stream.printError(current_node,
            "Unknown directive '" + directive + "'.");
        return;
      }
    }
  }
}

IncludeVisibility IncludePicker::ParseVisibility(
    const string& visibility) const {
  if (visibility == "private")
    return kPrivate;
  else if (visibility == "public")
    return kPublic;

  return kUnusedVisibility;
}

IncludeVisibility IncludePicker::GetVisibility(
    const string& quoted_include) const {
  return GetOrDefault(
      filepath_visibility_map_, quoted_include, kUnusedVisibility);
}

}  // namespace include_what_you_use
