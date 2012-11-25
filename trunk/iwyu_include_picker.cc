//===--- iwyu_include_picker.cc - map to canonical #includes for iwyu -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_include_picker.h"

#include <stddef.h>                     // for size_t
#include <algorithm>                    // for find
// TODO(wan): make sure IWYU doesn't suggest <iterator>.
#include <iterator>                     // for find
// not hash_map: it's not as portable and needs hash<string>.
#include <map>                          // for map, map<>::mapped_type, etc
#include <ostream>
#include <string>                       // for string, basic_string, etc
#include <utility>                      // for pair, make_pair
#include <vector>                       // for vector, vector<>::iterator

#include "iwyu_path_util.h"
#include "iwyu_stl_util.h"
#include "iwyu_string_util.h"
#include "iwyu_verrs.h"
#include "port.h"  // for CHECK_

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Regex.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLParser.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/system_error.h"

using std::find;
using std::make_pair;
using std::map;
using std::pair;
using std::string;
using std::vector;

using llvm::MemoryBuffer;
using llvm::OwningPtr;
using llvm::SourceMgr;
using llvm::errs;
using llvm::yaml::MappingNode;
using llvm::yaml::Node;
using llvm::yaml::ScalarNode;
using llvm::yaml::SequenceNode;
using llvm::yaml::Stream;
using llvm::yaml::document_iterator;

namespace include_what_you_use {

namespace {

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
  for (Each<string> node(nodes); !node.AtEnd(); ++node) {
    // First insert the node itself, then all its kids.
    if (!ContainsKey(seen_nodes_and_children, *node)) {
      nodes_and_children.push_back(*node);
      seen_nodes_and_children.insert(*node);
    }
    if (const vector<string>* children = FindInMap(&m, *node)) {
      for (Each<string> child(children); !child.AtEnd(); ++child) {
        if (!ContainsKey(seen_nodes_and_children, *child)) {
          nodes_and_children.push_back(*child);
          seen_nodes_and_children.insert(*child);
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
    // third-party code sometimes has #include cycles (*cough* boost
    // *cough*).  Because we add many implicit third-party mappings,
    // we may add a cycle without meaning to.  The best we can do is
    // to ignore the mapping that causes the cycle.  Same with code
    // in /internal/.  We could CHECK-fail in such a case, but it's
    // probably better to just keep going.
    if (StartsWith(key, "\"third_party/") ||
        key.find("internal/") != string::npos) {
      VERRS(4) << "Ignoring a cyclical mapping involving " << key << "\n";
      return;
    }
  }
  if (status == kCalculating) {
    VERRS(0) << "Cycle in include-mapping:\n";
    for (size_t i = 0; i < node_stack->size(); ++i)
      VERRS(0) << "  " << (*node_stack)[i] << " ->\n";
    VERRS(0) << "  " << key << "\n";
    CHECK_(false && "Cycle in include-mapping");  // cycle is a fatal error
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
  for (Each<string> child(&node->second); !child.AtEnd(); ++child) {
    node_stack->push_back(*child);
    MakeNodeTransitive(filename_map, seen_nodes, node_stack, *child);
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
  for (Each<string, vector<string> > it(filename_map); !it.AtEnd(); ++it)
    MakeNodeTransitive(filename_map, &seen_nodes, &node_stack, it->first);
}

// Get a scalar value from a YAML node.
// Returns empty string if it's not of type ScalarNode.
string GetScalarValue(Node* node) {
  ScalarNode* scalar = llvm::dyn_cast<ScalarNode>(node);
  if (scalar == NULL)
    return string();

  llvm::SmallString<8> storage;
  return scalar->getValue(storage).str();
}

// Get a sequence value from a YAML node.
// Returns empty vector if it's not of type SequenceNode.
vector<string> GetSequenceValue(Node* node) {
  vector<string> result;

  SequenceNode* sequence = llvm::dyn_cast<SequenceNode>(node);
  if (sequence != NULL) {
    for (SequenceNode::iterator it = sequence->begin();
         it != sequence->end(); ++it) {
      result.push_back(GetScalarValue(&*it));
    }
  }

  return result;
}

// Build a diagnostic string for an error in a mapping file.
// TODO(kimgr): Try to fix YAML parser to be able to use proper diagnostic
// infrastructure, for colored output, etc.
string MappingDiag(const SourceMgr& source_manager,
    const string& filename, const Node& node, const char* message) {
  pair<unsigned, unsigned> printable_loc
    = source_manager.getLineAndColumn(node.getSourceRange().Start);

  string buf;
  llvm::raw_string_ostream os(buf);
  os << filename << ":"
    << printable_loc.first << ":" << printable_loc.second << ": "
    << message;

  return os.str();
}

}  // namespace

IncludePicker::IncludePicker()
    : symbol_include_map_(),
      filepath_include_map_(),
      filepath_visibility_map_(),
      quoted_includes_to_quoted_includers_(),
      has_called_finalize_added_include_lines_(false) {
}

void IncludePicker::MarkVisibility(
    const string& quoted_filepath_pattern,
    IncludePicker::Visibility vis) {
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't mutate anymore");

  // insert() leaves any old value alone, and only inserts if the key is new.
  filepath_visibility_map_.insert(make_pair(quoted_filepath_pattern, vis));
  CHECK_(filepath_visibility_map_[quoted_filepath_pattern] == vis)
      << " Same file seen with two different visibilities: "
      << quoted_filepath_pattern
      << " Old vis: "
      << filepath_visibility_map_[quoted_filepath_pattern]
      << " New vis: "
      << vis;
}

// AddDirectInclude lets us use some hard-coded rules to add filepath
// mappings at runtime.  It includes, for instance, mappings from
// 'project/internal/foo.h' to 'project/public/foo_public.h' in google
// code (Google hides private headers in /internal/, much like glibc
// hides them in /bits/.)
void IncludePicker::AddDirectInclude(const string& includer_filepath,
                                     const string& includee_filepath,
                                     const string& quoted_include_as_typed) {
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't mutate anymore");

  // Note: the includer may be a .cc file, which is unnecessary to add
  // to our map, but harmless.
  const string quoted_includer = ConvertToQuotedInclude(includer_filepath);
  const string quoted_includee = ConvertToQuotedInclude(includee_filepath);

  quoted_includes_to_quoted_includers_[quoted_includee].insert(quoted_includer);
  const pair<string, string> key(includer_filepath, includee_filepath);
  includer_and_includee_to_include_as_typed_[key] = quoted_include_as_typed;

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

void IncludePicker::AddMappingFileSearchPath(const string& path) {
  string absolute_path = MakeAbsolutePath(path);
  if (std::find(mapping_file_search_path_.begin(),
                mapping_file_search_path_.end(),
                absolute_path) == mapping_file_search_path_.end()) {
    VERRS(6) << "Adding mapping file search path: " << absolute_path << "\n";
    mapping_file_search_path_.push_back(absolute_path);
  }
}

void IncludePicker::AddMapping(const string& map_from, const string& map_to) {
  VERRS(4) << "Adding mapping from " << map_from << " to " << map_to << "\n";
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't mutate anymore");
  CHECK_(IsQuotedFilepathPattern(map_from)
         && "All map keys must be quoted filepaths or @ followed by regex");
  CHECK_(IsQuotedInclude(map_to) && "All map values must be quoted includes");
  filepath_include_map_[map_from].push_back(map_to);
}

void IncludePicker::AddIncludeMapping(const string& map_from,
                                      IncludePicker::Visibility from_visibility,
                                      const string& map_to,
                                      IncludePicker::Visibility to_visibility) {
  AddMapping(map_from, map_to);
  MarkVisibility(map_from, from_visibility);
  MarkVisibility(map_to, to_visibility);
}

void IncludePicker::AddSymbolMapping(const string& map_from,
                                     IncludePicker::Visibility from_visibility,
                                     const string& map_to,
                                     IncludePicker::Visibility to_visibility) {
  CHECK_(IsQuotedInclude(map_to) && "Map values must be quoted includes");
  symbol_include_map_[map_from].push_back(map_to);

  // Symbol-names are always marked as private (or GetPublicValues()
  // will self-map them, below).
  MarkVisibility(map_from, kPrivate);
  MarkVisibility(map_to, to_visibility);
}

void IncludePicker::MarkIncludeAsPrivate(const string& quoted_filepath_pattern) {
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't mutate anymore");
  CHECK_(IsQuotedFilepathPattern(quoted_filepath_pattern)
         && "MIAP takes a quoted filepath pattern");
  MarkVisibility(quoted_filepath_pattern, kPrivate);
}

void IncludePicker::AddFriendRegex(const string& includee,
                                   const string& friend_regex) {
  friend_to_headers_map_["@" + friend_regex].insert(includee);
}

namespace {
// Given a map keyed by quoted filepath patterns, return a vector
// containing the @-regexes among the keys.
template <typename MapType>
vector<string> ExtractKeysMarkedAsRegexes(const MapType& m) {
  vector<string> regex_keys;
  for (Each<typename MapType::value_type> it(&m); !it.AtEnd(); ++it) {
    if (StartsWith(it->first, "@"))
      regex_keys.push_back(it->first);
  }
  return regex_keys;
}
}  // namespace

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
  for (Each<string, set<string> > incmap(&quoted_includes_to_quoted_includers_);
       !incmap.AtEnd(); ++incmap) {
    const string& hdr = incmap->first;
    for (Each<string> it(&filepath_include_map_regex_keys); !it.AtEnd(); ++it) {
      const string& regex_key = *it;
      const vector<string>& map_to = filepath_include_map_[regex_key];
      // Enclose the regex in ^(...)$ for full match.
      llvm::Regex regex(std::string("^(" + regex_key.substr(1) + ")$"));
      if (regex.match(hdr.c_str(), NULL) && !ContainsValue(map_to, hdr)) {
        Extend(&filepath_include_map_[hdr], filepath_include_map_[regex_key]);
        MarkVisibility(hdr, filepath_visibility_map_[regex_key]);
      }
    }
    for (Each<string> it(&friend_to_headers_map_regex_keys);
         !it.AtEnd(); ++it) {
      const string& regex_key = *it;
      llvm::Regex regex(std::string("^(" + regex_key.substr(1) + ")$"));
      if (regex.match(hdr.c_str(), NULL)) {
        InsertAllInto(friend_to_headers_map_[regex_key],
                      &friend_to_headers_map_[hdr]);
      }
    }
  }
}

// We treat third-party code specially, since it's difficult to add
// iwyu pragmas to code we don't own.  Basically, what we do is trust
// the code authors when it comes to third-party code: if they
// #include x.h to get symbols from y.h, then assume that's how the
// third-party authors wanted it.  This boils down to the following
// rules:
// 1) If there's already a mapping for third_party/y.h, do not
//    add any implicit maps for it.
// 2) if not_third_party/x.{h,cc} #includes third_party/y.h,
//    assume y.h is supposed to be included directly, and do not
//    add any implicit maps for it.
// 3) Otherwise, if third_party/x.h #includes third_party/y.h,
//    add a mapping from y.h to x.h.  Unless y.h already has
//    a hard-coded visibility set, make y.h private.  This
//    means iwyu will never suggest adding y.h.
void IncludePicker::AddImplicitThirdPartyMappings() {
  set<string> third_party_headers_with_explicit_mappings;
  for (Each<IncludeMap::value_type>
           it(&filepath_include_map_); !it.AtEnd(); ++it) {
    if (IsThirdPartyFile(it->first))
      third_party_headers_with_explicit_mappings.insert(it->first);
  }

  set<string> headers_included_from_non_third_party;
  for (Each<string, set<string> >
           it(&quoted_includes_to_quoted_includers_); !it.AtEnd(); ++it) {
    for (Each<string> includer(&it->second); !includer.AtEnd(); ++includer) {
      if (!IsThirdPartyFile(*includer)) {
        headers_included_from_non_third_party.insert(it->first);
        break;
      }
    }
  }

  for (Each<string, set<string> >
           it(&quoted_includes_to_quoted_includers_); !it.AtEnd(); ++it) {
    const string& includee = it->first;
    if (!IsThirdPartyFile(includee) ||
        ContainsKey(third_party_headers_with_explicit_mappings, includee) ||
        ContainsKey(headers_included_from_non_third_party, includee)) {
      continue;
    }
    for (Each<string> includer(&it->second); !includer.AtEnd(); ++includer) {
      // From the 'if' statement above, we already know that includee
      // is not included from non-third-party code.
      CHECK_(IsThirdPartyFile(*includer) && "Why not nixed!");
      CHECK_(IsThirdPartyFile(includee) && "Why not nixed!");
      AddMapping(includee, *includer);
      if (GetVisibility(includee) == kUnusedVisibility) {
        MarkIncludeAsPrivate(includee);
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

  // The map keys may be regular expressions.  Match those to seen #includes now.
  ExpandRegexes();

  // We treat third-party code specially, since it's difficult to add
  // iwyu pragmas to code we don't own.
  AddImplicitThirdPartyMappings();

  // If a.h maps to b.h maps to c.h, we'd like an entry from a.h to c.h too.
  MakeMapTransitive(&filepath_include_map_);
  // Now that filepath_include_map_ is transitively closed, it's an
  // easy task to get the values of symbol_include_map_ closed too.
  // We can't use Each<>() because we need a non-const iterator.
  for (IncludePicker::IncludeMap::iterator it = symbol_include_map_.begin();
       it != symbol_include_map_.end(); ++it) {
    ExpandOnce(filepath_include_map_, &it->second);
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
  for (Each<string> it(values); !it.AtEnd(); ++it) {
    CHECK_(!StartsWith(*it, "@"));
    if (GetOrDefault(filepath_visibility_map_, *it, kPublic) == kPublic)
      retval.push_back(*it);
  }
  return retval;
}

string IncludePicker::MaybeGetIncludeNameAsWritten(
    const string& includer_filepath, const string& includee_filepath) const {
  const pair<string, string> key(includer_filepath, includee_filepath);
  // I want to use GetOrDefault here, but it has trouble deducing tpl args.
  const string* value = FindInMap(&includer_and_includee_to_include_as_typed_,
                                  key);
  return value ? *value : "";
}

error_code IncludePicker::TryReadMappingFile(
    const string& filename,
    OwningPtr<MemoryBuffer>& buffer) const {
  string absolute_path;
  if (IsAbsolutePath(filename)) {
    VERRS(5) << "Absolute mapping filename: " << filename << ".\n";
    absolute_path = filename;
  } else {
    VERRS(5) << "Relative mapping filename: " << filename << ". "
      << "Scanning search path.\n";
    // Scan search path
    for (Each<string> it(&mapping_file_search_path_); !it.AtEnd(); ++it) {
      string candidate = MakeAbsolutePath(*it, filename);
      if (llvm::sys::fs::exists(candidate)) {
        absolute_path = candidate;
        VERRS(5) << "Found mapping file: " << candidate << ".\n";
        break;
      }
    }
  }

  error_code error = MemoryBuffer::getFile(absolute_path, buffer);
  VERRS(5) << "Opened mapping file: " << filename << "? "
    << error.message() << "\n";
  return error;
}

vector<string> IncludePicker::GetCandidateHeadersForSymbol(
    const string& symbol) const {
  CHECK_(has_called_finalize_added_include_lines_ && "Must finalize includes");
  return GetPublicValues(symbol_include_map_, symbol);
}

vector<string> IncludePicker::GetCandidateHeadersForFilepath(
    const string& filepath) const {
  CHECK_(has_called_finalize_added_include_lines_ && "Must finalize includes");
  const string quoted_header = ConvertToQuotedInclude(filepath);
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
  const string quoted_includer = ConvertToQuotedInclude(including_filepath);
  const string quoted_includee = ConvertToQuotedInclude(included_filepath);
  const set<string>* headers_with_includer_as_friend =
      FindInMap(&friend_to_headers_map_, quoted_includer);
  if (headers_with_includer_as_friend != NULL &&
      ContainsKey(*headers_with_includer_as_friend, included_filepath)) {
    retval.push_back(quoted_includee);
  } else {
    retval = GetCandidateHeadersForFilepath(included_filepath);
    if (retval.size() == 1) {
      const string& quoted_header = retval[0];
      if (GetVisibility(quoted_header) == IncludePicker::kPrivate) {
        VERRS(0) << "Warning: "
                 << "No public header found to replace the private header "
                 << quoted_header << "\n";
      }
    }
  }

  // We'll have called ConvertToQuotedInclude on members of retval,
  // but sometimes we can do better -- if included_filepath is in
  // retval, the iwyu-preprocessor may have stored the quoted-include
  // as typed in including_filepath.  This is better to use than
  // ConvertToQuotedInclude because it avoids trouble when the same
  // file is accessible via different include search-paths, or is
  // accessed via a symlink.
  const string& quoted_include_as_typed
      = MaybeGetIncludeNameAsWritten(including_filepath, included_filepath);
  if (!quoted_include_as_typed.empty()) {
    vector<string>::iterator it = std::find(retval.begin(), retval.end(),
                                            quoted_includee);
    if (it != retval.end())
      *it = quoted_include_as_typed;
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
    for (Each<string> it(all_mappers); !it.AtEnd(); ++it) {
      if (*it == quoted_to)
        return true;
    }
  }
  return quoted_to == quoted_from;   // indentity mapping, why not?
}

// Parses a YAML/JSON file containing mapping directives of various types:
//  symbol   - symbol name -> quoted include
//  include  - private quoted include -> public quoted include
//  ref      - include mechanism for mapping files, to allow project-specific
//             groupings
// We use this to maintain mappings externally, to make it easier
// to update/adjust to local circumstances.
void IncludePicker::AddMappingsFromFile(const string& filename) {
  OwningPtr<MemoryBuffer> buffer;
  error_code error = TryReadMappingFile(filename, buffer);
  if (error) {
    errs() << "Cannot open mapping file '" << filename << "': "
      << error.message() << ".\n";
    return;
  }

  SourceMgr source_manager;
  Stream json_stream(buffer->getBuffer(), source_manager);

  document_iterator stream_begin = json_stream.begin();
  if (stream_begin == json_stream.end())
    return;

  // Get root sequence.
  Node* root = stream_begin->getRoot();
  SequenceNode *array = llvm::dyn_cast<SequenceNode>(root);
  if (array == NULL) {
    errs() << MappingDiag(source_manager, filename, *root,
      "Root element must be an array.\n");
    return;
  }

  for (SequenceNode::iterator it = array->begin(); it != array->end(); ++it) {
    Node& current_node = *it;

    // Every item must be a JSON object ("mapping" in YAML terms.)
    MappingNode* mapping = llvm::dyn_cast<MappingNode>(&current_node);
    if (mapping == NULL) {
      errs() << MappingDiag(source_manager, filename, current_node,
        "Mapping directives must be objects.\n");
      return;
    }

    for (MappingNode::iterator it = mapping->begin();
         it != mapping->end(); ++it) {
      // General form is { directive: <data> }.
      const string directive = GetScalarValue(it->getKey());

      if (directive == "symbol") {
        // Symbol mapping.
        vector<string> mapping = GetSequenceValue(it->getValue());
        if (mapping.size() != 4) {
          errs() << MappingDiag(source_manager, filename, current_node,
            "Symbol mapping expects a value on the form "
            "'[from, visibility, to, visibility]'.\n");
          return;
        }

        Visibility from_visibility = ParseVisibility(mapping[1]);
        if (from_visibility == kUnusedVisibility) {
          errs() << MappingDiag(source_manager, filename, current_node,
            "Unknown visibility '") << mapping[1] << "'.\n";
          return;
        }

        Visibility to_visibility = ParseVisibility(mapping[3]);
        if (to_visibility == kUnusedVisibility) {
          errs() << MappingDiag(source_manager, filename, current_node,
            "Unknown visibility '") << mapping[3] << "'.\n";
          return;
        }

        AddSymbolMapping(
          mapping[0],
          from_visibility,
          mapping[2],
          to_visibility);
      } else if (directive == "include") {
        // Include mapping.
        vector<string> mapping = GetSequenceValue(it->getValue());
        if (mapping.size() != 4) {
          errs() << MappingDiag(source_manager, filename, current_node,
            "Include mapping expects a value on the form "
            "'[from, visibility, to, visibility]'.\n");
          return;
        }

        Visibility from_visibility = ParseVisibility(mapping[1]);
        if (from_visibility == kUnusedVisibility) {
          errs() << MappingDiag(source_manager, filename, current_node,
            "Unknown visibility '") << mapping[1] << "'.\n";
          return;
        }

        Visibility to_visibility = ParseVisibility(mapping[3]);
        if (to_visibility == kUnusedVisibility) {
          errs() << MappingDiag(source_manager, filename, current_node,
            "Unknown visibility '") << mapping[3] << "'.\n";
          return;
        }

        AddIncludeMapping(
          mapping[0],
          from_visibility,
          mapping[2],
          to_visibility);
      } else if (directive == "ref") {
        // Mapping ref.
        string ref_file = GetScalarValue(it->getValue());
        if (ref_file.empty()) {
          errs() << MappingDiag(source_manager, filename, current_node,
            "Mapping ref expects a single filename value.\n");
          return;
        }

        // Add the path of the file we're currently processing
        // to the search path. Allows refs to be relative to referrer.
        AddMappingFileSearchPath(GetParentPath(filename));

        // Recurse.
        AddMappingsFromFile(ref_file);
      } else {
        errs() << MappingDiag(source_manager, filename, current_node,
          "Unknown directive '") << directive << "'.\n";
        return;
      }
    }
  }
}

IncludePicker::Visibility IncludePicker::ParseVisibility(
    const string& visibility) const {
  if (visibility == "private")
    return kPrivate;
  else if (visibility == "public")
    return kPublic;

  return kUnusedVisibility;
}

IncludePicker::Visibility IncludePicker::GetVisibility(
    const string& quoted_include) const {
  return GetOrDefault(
      filepath_visibility_map_, quoted_include, kUnusedVisibility);
}

}  // namespace include_what_you_use
