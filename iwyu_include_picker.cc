//===--- iwyu_include_picker.cpp - map to canonical #includes for iwyu ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Ported from csilvers@'s Javascript version in
// sanitize.js and include_what_you_use_systemmap.js.

// Sanitizes a header file path, yielding something that can be used with
// an #include (including surrounding ""'s or <>'s).  The input
// might be a system header file:
//    /usr/grte/v1//include/stdio.h
//    /usr/local/google/crosstool/v13-nightly-31840/gcc-4.4.0-glibc-2.3.6-grte/x86_64-unknown-linux-gnu/bin/../lib/gcc/x86_64-unknown-linux-gnu/4.4.0/../../../../x86_64-unknown-linux-gnu/include/c++/4.4.0/bits/stl_vector.h
//    ./tests/badinc-i2.h
//
// The output for these would be
//    <stdio.h>
//    <vector>
//    "tests/badinc-i2.h"
//
// Sometimes this sanitizing is just path-munging: for instance, for
// stdio.h or badinc-i2.h above. But other times we need to map from
// internal-only header files to the externally visible header files
// that users should be #including (for instance, for stl_vector.h).
//
// This mapping can be a bit complicated in the case an internal-only
// header file is exposed via several public header files, such as
// bits/hashtable.h being exposed through both <unordered_map> and
// <unordered_set>. In that case, we list both public header files as
// a possible mapping from hashtable.h, and at runtime decide which is
// better. The basic algorithm is: if the user has already #included
// one or the other in the relevant source file, that one is the
// better mapping. If not, look at the other iwyu error messages
// we're planning on giving for this source file. If any these error
// messages mention <unordered_map> (in this example) -- that is,
// we're going to iwyu-warn about <unordered_map> anyway -- map
// hashtable.h to <unordered_map>. Likewise if any existing warning
// mentions <unordered_set>. If neither of these techniques yields a
// good mapping, we just pick one arbitrarily.

#include "iwyu_include_picker.h"

#include <assert.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "iwyu_path_util.h"
#include "iwyu_stl_util.h"
#include "iwyu_string_util.h"
#include "iwyu_output.h" // for VERRS
#include "llvm/Support/Path.h"

using std::map;
using std::pair;
using std::string;
using std::vector;

namespace include_what_you_use {

namespace {

enum HeaderKind { kUserHeader, kSystemHeader };

string QuoteHeader(const string& path, HeaderKind kind) {
  return (kind == kSystemHeader) ? ("<" + path + ">") : ("\"" + path + "\"");
}

string UnquoteHeader(const string& quoted_include) {
  assert(!quoted_include.empty() && "Should never have an empty #include");
  string retval = quoted_include;
  if (retval[0] == '<') {
    assert(retval.length() > 2);   // <x> is the smallest #include
    assert(retval[retval.length() - 1] == '>');
    StripLeft(&retval, "<");
    StripRight(&retval, ">");
  } else if (retval[0] == '"') {
    assert(retval.length() > 2);   // "x" is the smallest #include
    assert(retval[retval.length() - 1] == '"');
    StripLeft(&retval, "\"");
    StripRight(&retval, "\"");
  }
  return retval;
}

// For simple cases -- the third-party map, the symbol map -- making an
// include-map is just a matter of converting an array of <key,value>
// pairs into a multimap.  (Note we preserve the ordering, since when
// one key maps to multiple values, when all else is equal we'll
// prefer the first key.)
//
// However, for some maps, the values can also be keys: when a header
// could be #included by a user directly, or could be exported via
// another header (for instance, a user can #include <istream>, but if
// they #include <iostream>, they should get <istream> for free.  So
// we recurse, and add all those new entries in place: wherever we see
// a foo -> <istream> mapping, we replace it with two mappings:
//    foo -> <istream>
//    foo -> <iostream>    // because <istream> maps to <iostream>
// (If the value has no <> or "" around it (a mapping 'foo -> istream'),
// it means the value is a private header, and should be entirely
// replaced, not just added to.  In that case, we just insert
// foo -> <iostream>, and not foo -> <istream>.)
// We have a MakeTransitiveIncludeMap for these maps.
template <size_t kCount>
IncludePicker::IncludeMap MakeIncludeMap(
    const IncludePicker::IncludeMapEntry (&entries)[kCount]) {
  IncludePicker::IncludeMap include_map;
  for (size_t i = 0; i < kCount; i++) {
    include_map.insert(pair<string,string>(entries[i].name, entries[i].path));
  }
  return include_map;
}

// Does DFS for a single key/value pair, recursing whenever the value
// is itself a key in the map, and putting the results in a vector of
// all values seen.  original_key is the root of the dfs tree,
// key_value is our current position.
void AugmentValuesForKey(
    const IncludePicker::IncludeMap& m,
    const string& original_key,
    const IncludePicker::IncludeMap::value_type& key_value,
    const set<string>& seen_keys,     // used to avoid recursion
    vector<string>* all_values) {
  assert(!Contains(seen_keys, key_value.first) && "Cycle in include-mapping");
  const string new_key = UnquoteHeader(key_value.second);
  if (new_key != key_value.second) {  // not mapping to a private header
    all_values->push_back(key_value.second);
  }
  if (new_key != key_value.first) {   // not a self-mapping ("set -> <set>")
    set<string> new_seen_keys(seen_keys);
    new_seen_keys.insert(key_value.first);    // update the stack
    for (IncludePicker::IncludeMap::const_iterator it = m.lower_bound(new_key);
         it != m.upper_bound(new_key); ++it) {
      AugmentValuesForKey(m, original_key, *it, new_seen_keys, all_values);
    }
  }
}

// This could be made much more efficient, but I don't see a need yet.
IncludePicker::IncludeMap MakeTransitiveIncludeMap(
    const IncludePicker::IncludeMap& basic_include_map) {
  IncludePicker::IncludeMap retval;
  vector<string> all_values_for_current_key;
  set<string> stack_of_keys_seen;
  for (IncludePicker::IncludeMap::const_iterator it = basic_include_map.begin();
       it != basic_include_map.end(); ) {
    const string current_key = it->first;
    AugmentValuesForKey(basic_include_map, current_key, *it, stack_of_keys_seen,
                        &all_values_for_current_key);
    ++it;
    if (it == basic_include_map.end() || it->first != current_key) {
      all_values_for_current_key = GetUniqueEntries(all_values_for_current_key);
      // Next key is going to be different, so finish processing this key.
      for (Each<string> v(&all_values_for_current_key); !v.AtEnd(); ++v)
        retval.insert(IncludePicker::IncludeMap::value_type(current_key, *v));
      all_values_for_current_key.clear();
      stack_of_keys_seen.clear();
    }
  }
  return retval;
}

IncludePicker::IncludeMap MakeCppIncludeMap() {
  // One 'internal' header can map to several public headers; iwyu is
  // free to choose any of them.  All else being equal, iwyu will
  // choose the first mapping for a header.  If one private header
  // maps to another, iwyu will follow the chain, and logically
  // replace the mapped-to header in the map with all the public
  // headers it eventually maps to.
  // Note the value has the <> or "" as appropriate, but the key
  // does not.  If the value is missing a <> or "", it means it's
  // a private header and should be re-mapped as described above.
  static const IncludePicker::IncludeMapEntry cpp_include_map[] = {
    // Generated by running
    //    ( cd /usr/crosstool/v12/gcc-4.3.1-glibc-2.3.6-grte/x86_64-unknown-linux-gnu/x86_64-unknown-linux-gnu/include/c++/4.3.1 && grep '^ *# *include' {ext/,tr1/,}* | perl -nle 'm/^([^:]+).*<([^>]+)>/ && print qq@    { "$2", "<$1>" },@' | grep -e bits/ -e tr1_impl/ | sort -u)
    // and then taking the 'meaningful' dependencies -- for instance,
    // <functional> #includes <bits/stringfwd.h>, but if someone is
    // using strings, <functional> isn't enough to satisfy iwyu.
    // It may be that for future libc versions, we will want dirs
    // other than bits/.
    { "bits/algorithmfwd.h", "<algorithm>" },
    { "bits/allocator.h", "<memory>" },
    { "bits/atomic_word.h", "<ext/atomicity.h>" },
    { "bits/basic_file.h", "<fstream>" },
    { "bits/basic_ios.h", "<ios>" },
    { "bits/basic_string.h", "<string>" },
    { "bits/basic_string.tcc", "<string>" },
    { "bits/boost_sp_shared_count.h", "<memory>" },
    { "bits/c++io.h", "<ext/stdio_sync_filebuf.h>" },
    { "bits/c++config.h", "<cstddef>" },
    { "bits/char_traits.h", "<string>" },
    { "bits/cmath.tcc", "<cmath>" },
    { "bits/codecvt.h", "<fstream>" },
    { "bits/cxxabi_tweaks.h", "<cxxabi.h>" },
    { "bits/deque.tcc", "<deque>" },
    { "bits/fstream.tcc", "<fstream>" },
    { "bits/functional_hash.h", "<unordered_map>" },
    { "bits/gslice.h", "<valarray>" },
    { "bits/gslice_array.h", "<valarray>" },
    { "bits/hashtable.h", "<unordered_map>" },
    { "bits/hashtable.h", "<unordered_set>" },
    { "bits/indirect_array.h", "<valarray>" },
    { "bits/ios_base.h", "<iostream>" },
    { "bits/ios_base.h", "<ios>" },
    { "bits/ios_base.h", "<iomanip>" },
    { "bits/locale_classes.h", "<locale>" },
    { "bits/locale_facets.h", "<locale>" },
    { "bits/locale_facets_nonio.h", "<locale>" },
    { "bits/localefwd.h", "<locale>" },
    { "bits/mask_array.h", "<valarray>" },
    { "bits/ostream.tcc", "<ostream>" },
    { "bits/ostream_insert.h", "<ostream>" },
    { "bits/postypes.h", "<iostream>" },
    { "bits/slice_array.h", "<valarray>" },
    { "bits/stl_algo.h", "<algorithm>" },
    { "bits/stl_algobase.h", "<algorithm>" },
    { "bits/stl_bvector.h", "<vector>" },
    { "bits/stl_construct.h", "<memory>" },
    { "bits/stl_deque.h", "<deque>" },
    { "bits/stl_function.h", "<functional>" },
    { "bits/stl_heap.h", "<queue>" },
    { "bits/stl_iterator.h", "<iterator>" },
    { "bits/stl_iterator_base_funcs.h", "<iterator>" },
    { "bits/stl_iterator_base_types.h", "<iterator>" },
    { "bits/stl_list.h", "<list>" },
    { "bits/stl_map.h", "<map>" },
    { "bits/stl_multimap.h", "<map>" },
    { "bits/stl_multiset.h", "<set>" },
    { "bits/stl_numeric.h", "<numeric>" },
    { "bits/stl_pair.h", "<utility>"},
    { "bits/stl_pair.h", "<tr1/utility>" },
    { "bits/stl_queue.h", "<queue>" },
    { "bits/stl_raw_storage_iter.h", "<memory>" },
    { "bits/stl_relops.h", "<utility>" },
    { "bits/stl_set.h", "<set>" },
    { "bits/stl_stack.h", "<stack>" },
    { "bits/stl_tempbuf.h", "<memory>" },
    { "bits/stl_tree.h", "<map>" },
    { "bits/stl_tree.h", "<set>" },
    { "bits/stl_uninitialized.h", "<memory>" },
    { "bits/stl_vector.h", "<vector>" },
    { "bits/stream_iterator.h", "<iterator>" },
    { "bits/streambuf.tcc", "<streambuf>" },
    { "bits/streambuf_iterator.h", "<iterator>" },
    { "bits/stringfwd.h", "<string>" },
    { "bits/valarray_array.h", "<valarray>" },
    { "bits/valarray_before.h", "<valarray>" },
    { "bits/vector.tcc", "<vector>" },
    { "tr1_impl/array", "<array>" },
    { "tr1_impl/array", "<tr1/array>" },
    { "tr1_impl/boost_shared_ptr.h", "<memory>" },
    { "tr1_impl/boost_shared_ptr.h", "<tr1/memory>" },
    { "tr1_impl/boost_sp_counted_base.h", "<memory>" },
    { "tr1_impl/boost_sp_counted_base.h", "<tr1/memory>" },
    { "tr1_impl/cctype", "<cctype>" },
    { "tr1_impl/cctype", "<tr1/cctype>" },
    { "tr1_impl/cfenv", "<cfenv>" },
    { "tr1_impl/cfenv", "<tr1/cfenv>" },
    { "tr1_impl/cinttypes", "<cinttypes>" },
    { "tr1_impl/cinttypes", "<tr1/cinttypes>" },
    { "tr1_impl/cmath", "<cmath>" },
    { "tr1_impl/cmath", "<tr1/cmath>" },
    { "tr1_impl/complex", "<complex>" },
    { "tr1_impl/complex", "<tr1/complex>" },
    { "tr1_impl/cstdint", "<cstdint>" },
    { "tr1_impl/cstdint", "<tr1/cstdint>" },
    { "tr1_impl/cstdio", "<cstdio>" },
    { "tr1_impl/cstdio", "<tr1/cstdio>" },
    { "tr1_impl/cstdlib", "<cstdlib>" },
    { "tr1_impl/cstdlib", "<tr1/cstdlib>" },
    { "tr1_impl/cwchar", "<cwchar>" },
    { "tr1_impl/cwchar", "<tr1/cwchar>" },
    { "tr1_impl/cwctype", "<cwctype>" },
    { "tr1_impl/cwctype", "<tr1/cwctype>" },
    { "tr1_impl/functional", "<functional>" },
    { "tr1_impl/functional", "<tr1/functional>" },
    { "tr1_impl/functional_hash.h", "<tr1/functional_hash.h>" },
    { "tr1_impl/hashtable", "<tr1/hashtable.h>" },
    { "tr1_impl/random", "<random>" },
    { "tr1_impl/random", "<tr1/random>" },
    { "tr1_impl/regex", "<regex>" },
    { "tr1_impl/regex", "<tr1/regex>" },
    { "tr1_impl/type_traits", "<tr1/type_traits>" },
    { "tr1_impl/type_traits", "<type_traits>" },
    { "tr1_impl/unordered_map", "<tr1/unordered_map>" },
    { "tr1_impl/unordered_map", "<unordered_map>" },
    { "tr1_impl/unordered_set", "<tr1/unordered_set>" },
    { "tr1_impl/unordered_set", "<unordered_set>" },
    { "tr1_impl/utility", "<tr1/utility>" },
    { "tr1_impl/utility", "<utility>" },
    // This didn't come from the grep, but seems to be where swap()
    // is defined?
    { "bits/move.h", "<algorithm>" },   // for swap<>()
    // All .tcc files are gcc internal-include files.  We get them from
    // ( cd /usr/crosstool/v12/gcc-4.3.1-glibc-2.3.6-grte/x86_64-unknown-linux-gnu/x86_64-unknown-linux-gnu/include/c++/4.3.1 && grep -R '^ *# *include.*tcc' * | perl -nle 'm/^([^:]+).*[<"]([^>"]+)[>"]/ && print qq@    { "$2", "<$1>" },@' | sort )
    // Note some of the mapped-to values are themselves private headers,
    // so I had to manually edit them to remove the <>'s around them.
    { "bits/basic_ios.tcc", "bits/basic_ios.h" },
    { "bits/basic_string.tcc", "<string>" },
    { "bits/cmath.tcc", "<cmath>" },
    { "bits/deque.tcc", "<deque>" },
    { "bits/fstream.tcc", "<fstream>" },
    { "bits/istream.tcc", "<istream>" },
    { "bits/list.tcc", "<list>" },
    { "bits/locale_classes.tcc", "bits/locale_classes.h" },  // private->private
    { "bits/locale_facets.tcc", "bits/locale_facets.h" },
    { "bits/locale_facets_nonio.tcc", "bits/locale_facets_nonio.h" },
    { "bits/ostream.tcc", "<ostream>" },
    { "bits/sstream.tcc", "<sstream>" },
    { "bits/streambuf.tcc", "<streambuf>" },
    { "bits/valarray_array.tcc", "bits/valarray_array.h" },
    { "bits/vector.tcc", "<vector>" },
    { "debug/safe_iterator.tcc", "<debug/safe_iterator.h>" },
    { "tr1/bessel_function.tcc", "<tr1/cmath>" },
    { "tr1/beta_function.tcc", "<tr1/cmath>" },
    { "tr1/ell_integral.tcc", "<tr1/cmath>" },
    { "tr1/exp_integral.tcc", "<tr1/cmath>" },
    { "tr1/gamma.tcc", "<tr1/cmath>" },
    { "tr1/hypergeometric.tcc", "<tr1/cmath>" },
    { "tr1/legendre_function.tcc", "<tr1/cmath>" },
    { "tr1/modified_bessel_func.tcc", "<tr1/cmath>" },
    { "tr1/poly_hermite.tcc", "<tr1/cmath>" },
    { "tr1/poly_laguerre.tcc", "<tr1/cmath>" },
    { "tr1/riemann_zeta.tcc", "<tr1/cmath>" },
    { "tr1_impl/random.tcc", "tr1_impl/random" },
    // Some bits->bits #includes.  A few files in bits re-export
    // symbols from other files in bits.  To find these, I ran
    // ( cd /usr/crosstool/v12/gcc-4.3.1-glibc-2.3.6-grte/x86_64-unknown-linux-gnu/x86_64-unknown-linux-gnu/include/c++/4.3.1 && grep '^ *# *include.*bits/' bits/* | perl -nle 'm/^([^:]+).*<([^>]+)>/ && print qq@    { "$2", "$1" },@' | grep bits/ | sort -u)
    // and carefully picked reasonable-looking results (algorithm
    // *uses* pair but doesn't *re-export* pair, for instance).
    { "bits/boost_concept_check.h", "bits/concept_check.h" },
    { "bits/c++allocator.h", "bits/allocator.h" },
    { "bits/codecvt.h", "bits/locale_facets_nonio.h" },
    { "bits/functexcept.h", "bits/stl_algobase.h" },
    { "bits/locale_classes.h", "bits/basic_ios.h" },
    { "bits/locale_facets.h", "bits/basic_ios.h" },
    { "bits/messages_members.h", "bits/locale_facets_nonio.h" },
    { "bits/postypes.h", "bits/char_traits.h" },
    { "bits/slice_array.h", "bits/valarray_before.h" },
    { "bits/stl_construct.h", "bits/stl_tempbuf.h" },
    { "bits/stl_move.h", "bits/stl_algobase.h" },
    { "bits/stl_uninitialized.h", "bits/stl_tempbuf.h" },
    { "bits/stl_vector.h", "bits/stl_bvector.h" },
    { "bits/streambuf_iterator.h", "bits/basic_ios.h" },
    // I don't think we want to be having people move to 'backward/'
    // yet.  (These hold deprecated STL classes that we still use
    // actively.)  These are the ones that turned up in an analysis of
    { "backward/binders.h", "<functional>" },
    { "backward/hash_fun.h", "<hash_map>" },
    { "backward/hash_fun.h", "<hash_set>" },
    { "backward/hashtable.h", "<hash_map>" },
    { "backward/hashtable.h", "<hash_set>" },
    { "backward/strstream", "<strstream>" },
    // We do our own string implementation, which needs some mappings.
    { "ext/vstring_fwd.h", "<string>" },
    { "ext/vstring.h", "<string>" },
    { "ext/vstring.tcc", "<string>" },
    // (This one should perhaps be found automatically somehow.)
    { "ext/sso_string_base.h", "<string>" },
    { "ext/hash_set", "<hash_set>" },
    { "ext/hash_map", "<hash_map>" },
    { "ext/slist", "<slist>" },
    // The iostream .h files are confusing.  Lots of private headers,
    // which are handled above, but we also have public headers
    // #including each other (eg <iostream> #includes <istream>).  We
    // are pretty forgiving: if a user specifies any public header, we
    // generally don't require the others.  Do those mappings here.
    // Note that since these are all public headers we're mapping, we
    // need to map them to themselves to say they're ok as they are.
    // We also need self-mappings to tell iwyu: "if the user #included
    // <ios>, you don't *need* to map it to something else if you
    // don't want to."  I got these via
    // ( cd /usr/crosstool/v12/gcc-4.3.1-glibc-2.3.6-grte/x86_64-unknown-linux-gnu/x86_64-unknown-linux-gnu/include/c++/4.3.1 && egrep '^ *# *include <(istream|ostream|iostream|fstream|sstream|streambuf|ios)>' *stream* ios | perl -nle 'm/^([^:]+).*[<"]([^>"]+)[>"]/ and print qq@    { "$2", "<$1>" },@ and print qq@    { "$2", "<$2>" },@' | sort -u )
    { "ios", "<ios>" },
    { "ios", "<istream>" },
    { "ios", "<ostream>" },
    { "istream", "<fstream>" },
    { "istream", "<iostream>" },
    { "istream", "<istream>" },
    { "istream", "<sstream>" },
    { "ostream", "<fstream>" },
    { "ostream", "<iostream>" },
    { "ostream", "<istream>" },
    { "ostream", "<ostream>" },
    { "ostream", "<sstream>" },
    { "streambuf", "<ios>" },
    { "streambuf", "<streambuf>" },
    // When you use logging.h, you also typically use << to emit a log
    // message.  But you don't need to bring in ostream or string
    // yourself to use <<.  logging.h gives it to you.  So if you
    // #include base/logging.h, don't complain you also need ostream.
    // (This is low in the list because we prefer iwyu use the ostream
    // mappings above if it can -- this mapping may be wrong at times.)
    { "ostream", "\"base/logging.h\"" },
  };

  return MakeTransitiveIncludeMap(MakeIncludeMap(cpp_include_map));
}

IncludePicker::IncludeMap MakeCIncludeMap() {
  // For C includes, we can use the same idea we used for C++ includes.  For
  // this one, we ran
  //   ( cd /usr/include && grep '^ *# *include' {sys/,net/,}* | perl -nle 'm/^([^:]+).*<([^>]+)>/ && print qq@    { "$2", "<$1>" },@' | grep bits/ | sort )
  const IncludePicker::IncludeMapEntry c_include_map[] = {
    { "bits/a.out.h", "<a.out.h>" },
    { "bits/byteswap.h", "<byteswap.h>" },
    { "bits/cmathcalls.h", "<complex.h>" },
    { "bits/confname.h", "<unistd.h>" },
    { "bits/dirent.h", "<dirent.h>" },
    { "bits/dlfcn.h", "<dlfcn.h>" },
    { "bits/elfclass.h", "<link.h>" },
    { "bits/endian.h", "<endian.h>" },
    { "bits/environments.h", "<unistd.h>" },
    { "bits/errno.h", "<errno.h>" },
    { "bits/error.h", "<error.h>" },
    { "bits/fcntl.h", "<fcntl.h>" },
    { "bits/fcntl2.h", "<fcntl.h>" },
    { "bits/fenv.h", "<fenv.h>" },
    { "bits/fenvinline.h", "<fenv.h>" },
    { "bits/huge_val.h", "<math.h>" },
    { "bits/huge_valf.h", "<math.h>" },
    { "bits/huge_vall.h", "<math.h>" },
    { "bits/ioctl-types.h", "<sys/ioctl.h>" },
    { "bits/ioctls.h", "<sys/ioctl.h>" },
    { "bits/ipc.h", "<sys/ipc.h>" },
    { "bits/ipctypes.h", "<sys/ipc.h>" },
    { "bits/libio-ldbl.h", "<libio.h>" },
    { "bits/link.h", "<link.h>" },
    { "bits/locale.h", "<locale.h>" },
    { "bits/mathcalls.h", "<math.h>" },
    { "bits/mathdef.h", "<math.h>" },
    { "bits/mman.h", "<sys/mman.h>" },
    { "bits/monetary-ldbl.h", "<monetary.h>" },
    { "bits/mqueue.h", "<mqueue.h>" },
    { "bits/mqueue2.h", "<mqueue.h>" },
    { "bits/msq.h", "<sys/msg.h>" },
    { "bits/nan.h", "<math.h>" },
    { "bits/netdb.h", "<netdb.h>" },
    { "bits/poll.h", "<sys/poll.h>" },
    { "bits/posix1_lim.h", "<limits.h>" },
    { "bits/posix2_lim.h", "<limits.h>" },
    { "bits/posix_opt.h", "<unistd.h>" },
    { "bits/printf-ldbl.h", "<printf.h>" },
    { "bits/pthreadtypes.h", "<pthread.h>" },
    { "bits/resource.h", "<sys/resource.h>" },
    { "bits/sched.h", "<sched.h>" },
    { "bits/select.h", "<sys/select.h>" },
    { "bits/sem.h", "<sys/sem.h>" },
    { "bits/semaphore.h", "<semaphore.h>" },
    { "bits/setjmp.h", "<setjmp.h>" },
    { "bits/shm.h", "<sys/shm.h>" },
    { "bits/sigaction.h", "<signal.h>" },
    { "bits/sigcontext.h", "<signal.h>" },
    { "bits/siginfo.h", "<signal.h>" },
    { "bits/signum.h", "<signal.h>" },
    { "bits/sigset.h", "<signal.h>" },
    { "bits/sigstack.h", "<signal.h>" },
    { "bits/sigthread.h", "<signal.h>" },
    { "bits/sockaddr.h", "<sys/un.h>" },
    { "bits/socket.h", "<sys/socket.h>" },
    { "bits/stab.def", "<stab.h>" },
    { "bits/stat.h", "<sys/stat.h>" },
    { "bits/statfs.h", "<sys/statfs.h>" },
    { "bits/statvfs.h", "<sys/statvfs.h>" },
    { "bits/stdio-ldbl.h", "<stdio.h>" },
    { "bits/stdio-lock.h", "<libio.h>" },
    { "bits/stdio.h", "<stdio.h>" },
    { "bits/stdio2.h", "<stdio.h>" },
    { "bits/stdio_lim.h", "<stdio.h>" },
    { "bits/stdlib-ldbl.h", "<stdlib.h>" },
    { "bits/stdlib.h", "<stdlib.h>" },
    { "bits/string.h", "<string.h>" },
    { "bits/string2.h", "<string.h>" },
    { "bits/string3.h", "<string.h>" },
    { "bits/stropts.h", "<stropts.h>" },
    { "bits/sys_errlist.h", "<stdio.h>" },
    { "bits/syscall.h", "<sys/syscall.h>" },
    { "bits/syslog-ldbl.h", "<sys/syslog.h>" },
    { "bits/syslog-path.h", "<sys/syslog.h>" },
    { "bits/syslog.h", "<sys/syslog.h>" },
    { "bits/termios.h", "<termios.h>" },
    { "bits/time.h", "<sys/time.h>" },
    { "bits/types.h", "<sys/types.h>" },
    { "bits/uio.h", "<sys/uio.h>" },
    { "bits/unistd.h", "<unistd.h>" },
    { "bits/ustat.h", "<sys/ustat.h>" },
    { "bits/utmp.h", "<utmp.h>" },
    { "bits/utmpx.h", "<utmpx.h>" },
    { "bits/utsname.h", "<sys/utsname.h>" },
    { "bits/waitflags.h", "<sys/wait.h>" },
    { "bits/waitstatus.h", "<sys/wait.h>" },
    { "bits/wchar-ldbl.h", "<wchar.h>" },
    { "bits/wchar.h", "<wchar.h>" },
    { "bits/wchar2.h", "<wchar.h>" },
    { "bits/xopen_lim.h", "<limits.h>" },
    { "bits/xtitypes.h", "<stropts.h>" },
    // Sometimes libc tells you what mapping to do via an '#error':
    // # error "Never use <bits/dlfcn.h> directly; include <dlfcn.h> instead."
    //    ( cd /usr/include && grep -R '^ *# *error "Never use' * | perl -nle 'm/<([^>]+).*<([^>]+)/ && print qq@    { "$1", "<$2>" },@' | sort )
    { "bits/a.out.h", "<a.out.h>" },
    { "bits/byteswap.h", "<byteswap.h>" },
    { "bits/cmathcalls.h", "<complex.h>" },
    { "bits/confname.h", "<unistd.h>" },
    { "bits/dirent.h", "<dirent.h>" },
    { "bits/dlfcn.h", "<dlfcn.h>" },
    { "bits/elfclass.h", "<link.h>" },
    { "bits/endian.h", "<endian.h>" },
    { "bits/fcntl.h", "<fcntl.h>" },
    { "bits/fenv.h", "<fenv.h>" },
    { "bits/huge_val.h", "<math.h>" },
    { "bits/huge_valf.h", "<math.h>" },
    { "bits/huge_vall.h", "<math.h>" },
    { "bits/in.h", "<netinet/in.h>" },
    { "bits/inf.h", "<math.h>" },
    { "bits/ioctl-types.h", "<sys/ioctl.h>" },
    { "bits/ioctls.h", "<sys/ioctl.h>" },
    { "bits/ipc.h", "<sys/ipc.h>" },
    { "bits/locale.h", "<locale.h>" },
    { "bits/mathdef.h", "<math.h>" },
    { "bits/mathinline.h", "<math.h>" },
    { "bits/mman.h", "<sys/mman.h>" },
    { "bits/mqueue.h", "<mqueue.h>" },
    { "bits/msq.h", "<sys/msg.h>" },
    { "bits/nan.h", "<math.h>" },
    { "bits/poll.h", "<sys/poll.h>" },
    { "bits/predefs.h", "<features.h>" },
    { "bits/resource.h", "<sys/resource.h>" },
    { "bits/select.h", "<sys/select.h>" },
    { "bits/semaphore.h", "<semaphore.h>" },
    { "bits/sigcontext.h", "<signal.h>" },
    { "bits/string.h", "<string.h>" },
    { "bits/string2.h", "<string.h>" },
    { "bits/string3.h", "<string.h>" },
    { "bits/syscall.h", "<sys/syscall.h>" },
    // Here are the top-level #includes that just forward to another
    // file:
    //    $ for i in /usr/include/*; do [ -f $i ] && [ `wc -l < $i` = 1 ] && echo $i; done
    // (poll.h, syscall.h, syslog.h, ustat.h, wait.h).
    // For each file, I looked at the list of canonical header files --
    // http://www.opengroup.org/onlinepubs/9699919799/idx/head.html --
    // to decide which of the two files is canonical.  If neither is
    // on the POSIX.1 1998 list, I just choose the top-level one.
    { "sys/poll.h", "<poll.h>" },
    { "sys/syscall.h", "<syscall.h>" },
    { "sys/syslog.h", "<syslog.h>" },
    { "sys/ustat.h", "<ustat.h>" },
    { "wait.h", "<sys/wait.h>" },
    // These are all files in bits/ that delegate to asm/ and linux/ to
    // do all (or lots) of the work.
    // $ for i in /usr/include/bits/*; do for dir in asm linux; do grep -H -e $dir/`basename $i` $i; done; done
    { "linux/errno.h", "bits/errno.h" },   // map to a private header file
    { "asm/ioctls.h", "bits/ioctls.h" },
    { "asm/socket.h", "bits/socket.h" },
    { "linux/socket.h", "bits/socket.h" },
    // Some asm files have 32- and 64-bit variants:
    // $ ls /usr/include/asm/*_{32,64}.h
    { "asm/posix_types_32.h", "<asm/posix_types.h>" },
    { "asm/posix_types_64.h", "<asm/posix_types.h>" },
    { "asm/unistd_32.h", "asm/unistd.h" },
    { "asm/unistd_64.h", "asm/unistd.h" },
    // I don't know what grep would have found these.  I found them
    // via user report.
    { "asm/errno.h", "<errno.h>" },       // also captures asm-generic/errno.h
    { "asm/errno-base.h", "<errno.h>" },  // actually asm-generic/errno-base.h
    { "asm/ptrace-abi.h", "<asm/ptrace.h>" },
    { "asm/unistd.h", "<syscall.h>" },
    { "linux/limits.h", "<limits.h>" },   // where PATH_MAX is defined
    { "linux/prctl.h", "<sys/prctl.h>" },
    { "sys/ucontext.h", "<ucontext.h>" },
    // Allow the C++ wrappers around C files.  Without these mappings,
    // if you #include <cstdio>, iwyu will tell you to replace it with
    // <stdio.h>, which is where the symbols are actually defined.  We
    // inhibit that behavior to keep the <cstdio> alone.  Note we do a
    // self-mapping first, because we prefer keeping the #include at
    // <stdio.h> rather than mapping to it <cstdio>, all else being
    // equal.  Here is how I identified the files to map:
    // $ for i in /usr/include/c++/4.4/c* ; do ls /usr/include/`basename $i | cut -b2-`.h 2>/dev/null ; done
    { "assert.h", "<assert.h>" },
    { "assert.h", "<cassert>" },
    { "complex.h", "<complex.h>" },
    { "complex.h", "<ccomplex>" },
    { "ctype.h", "<ctype.h>" },
    { "ctype.h", "<cctype>" },
    { "errno.h", "<errno.h>" },
    { "errno.h", "<cerrno>" },
    { "fenv.h", "<fenv.h>" },
    { "fenv.h", "<cfenv>" },
    { "inttypes.h", "<inttypes.h>" },
    { "inttypes.h", "<cinttypes>" },
    { "limits.h", "<limits.h>" },
    { "limits.h", "<climits>" },
    { "locale.h", "<locale.h>" },
    { "locale.h", "<clocale>" },
    { "math.h", "<math.h>" },
    { "math.h", "<cmath>" },
    { "setjmp.h", "<setjmp.h>" },
    { "setjmp.h", "<csetjmp>" },
    { "signal.h", "<signal.h>" },
    { "signal.h", "<csignal>" },
    { "stdint.h", "<stdint.h>" },
    { "stdint.h", "<cstdint>" },
    { "stdio.h", "<stdio.h>" },
    { "stdio.h", "<cstdio>" },
    { "stdlib.h", "<stdlib.h>" },
    { "stdlib.h", "<cstdlib>" },
    { "string.h", "<string.h>" },
    { "string.h", "<cstring>" },
    { "tgmath.h", "<tgmath.h>" },
    { "tgmath.h", "<ctgmath>" },
    { "time.h", "<time.h>" },
    { "time.h", "<ctime>" },
    { "wchar.h", "<wchar.h>" },
    { "wchar.h", "<cwchar>" },
    { "wctype.h", "<wctype.h>" },
    { "wctype.h", "<cwctype>" },
    { "ieee754.h", "\"base/port_ieee.h\"" },
    // TODO(csilvers): map portable stuff to port.h (or rewrite port.h?)
    // eg bswap_16/32/64, strtoq/uq/ll,ull, atoll, etc.
  };

  return MakeTransitiveIncludeMap(MakeIncludeMap(c_include_map));
}

IncludePicker::IncludeMap MakeGoogleIncludeMap() {
  // The same idea for C++ includes can be used for the occasional
  const IncludePicker::IncludeMapEntry google_include_map[] = {
    // These two are here just for unittesting.
    { "tests/badinc-private.h",
      "\"tests/badinc-inl.h\"" },
    { "tests/badinc-private2.h",
      "\"tests/badinc-inl.h\"" },
    { "base/atomicops-internals-macosx.h", "\"base/atomicops.h\"" },
    { "base/atomicops-internals-x86-msvc.h", "\"base/atomicops.h\"" },
    { "base/atomicops-internals-x86.h", "\"base/atomicops.h\"" },
    { "base/callback-specializations.h", "\"base/callback.h\"" },
    // We prefer callback-types.h to callback.h, if we're told to add
    // one or the other, but if we need callback.h for some other
    // reason, we don't need to then add callback-types.h on top of that.
    { "base/callback-types.h", "\"base/callback-types.h\"" },
    { "base/callback-types.h", "\"base/callback.h\"" },
    { "i18n/encodings/proto/encodings.pb.h",
      "\"i18n/encodings/public/encodings.h\"" },
    { "i18n/languages/proto/languages.pb.h",
      "\"i18n/languages/public/languages.h\"" },
    { "net/proto/proto-array-internal.h",
      "\"net/proto/proto-array.h\"" },
    { "testing/base/public/gmock_utils/protocol-buffer-matchers.h",
      "\"testing/base/public/gmock.h\"" },
    { "util/gtl/container_literal_generated.h",
      "\"util/gtl/container_literal.h\"" },
    { "util/bits/bits-internal-unknown.h", "\"util/bits/bits.h\"" },
    { "util/bits/bits-internal-windows.h", "\"util/bits/bits.h\"" },
    { "util/bits/bits-internal-x86.h", "\"util/bits/bits.h\"" },
    // These are .h files that aren't private, but are subsumed by
    // another .h.  For instance, logging.h and raw_logging.h
    // re-export all of log_severity's symbols.  So if someone
    // #includes logging.h, they don't need to #include log_severity.h
    // as well.  But it's ok if they do, and it's ok to #include
    // log_severity.h without #including logging.h.
    { "base/log_severity.h", "\"base/log_severity.h\"" },
    { "base/log_severity.h", "\"base/logging.h\"" },
    { "base/log_severity.h", "\"base/raw_logging.h\"" },
    { "base/vlog_is_on.h", "\"base/vlog_is_on.h\"" },
    { "base/vlog_is_on.h", "\"base/logging.h\"" },
    { "base/vlog_is_on.h", "\"base/raw_logging.h\"" },
    { "stats/io/public/ev_doc.h", "\"stats/io/public/ev_doc.h\"" },
    { "stats/io/public/ev_doc.h", "\"stats/io/public/expvar.h\"" },
    { "stats/io/public/ev_doc.h", "\"stats/io/public/stats.h\"" },
    { "stats/io/public/ev_doc.h", "\"stats/io/public/exporter.h\"" },
  };

  return MakeTransitiveIncludeMap(MakeIncludeMap(google_include_map));
}

IncludePicker::IncludeMap MakeSymbolIncludeMap() {
  // For library symbols that can be defined in more than one header
  // file, maps from symbol-name to legitimate header files.
  // This list was generated via
  //    grep -R '__.*_defined' /usr/include | perl -nle 'm,/usr/include/([^:]*):#\s*\S+ __(.*)_defined, and print qq@    { "$2", "<$1>" },@' | sort -u
  // I ignored all entries that only appeared once on the list (eg uint32_t).
  // I then added in NULL, which according to [diff.null] C.2.2.3, can
  // be defined in <clocale>, <cstddef>, <cstdio>, <cstdlib>,
  // <cstring>, <ctime>, or <cwchar>.  We also allow their C
  // equivalents.
  // In each case, I ordered them so <sys/types.h> was first, if it was
  // an option for this type.  That's the preferred #include all else equal.
  const IncludePicker::IncludeMapEntry symbol_include_map[] = {
    { "blksize_t", "<sys/types.h>" },
    { "blkcnt_t", "<sys/stat.h>" },
    { "blkcnt_t", "<sys/types.h>" },
    { "blksize_t", "<sys/stat.h>" },
    { "daddr_t", "<sys/types.h>" },
    { "daddr_t", "<rpc/types.h>" },
    { "dev_t", "<sys/types.h>" },
    { "dev_t", "<sys/stat.h>" },
    { "error_t", "<errno.h>" },
    { "error_t", "<argp.h>" },
    { "error_t", "<argz.h>" },
    { "fsblkcnt_t", "<sys/types.h>" },
    { "fsblkcnt_t", "<sys/statvfs.h>" },
    { "fsfilcnt_t", "<sys/types.h>" },
    { "fsfilcnt_t", "<sys/statvfs.h>" },
    { "gid_t", "<sys/types.h>" },
    { "gid_t", "<grp.h>" },
    { "gid_t", "<pwd.h>" },
    { "gid_t", "<stropts.h>" },
    { "gid_t", "<sys/ipc.h>" },
    { "gid_t", "<sys/stat.h>" },
    { "gid_t", "<unistd.h>" },
    { "id_t", "<sys/types.h>" },
    { "id_t", "<sys/resource.h>" },
    { "ino64_t", "<sys/types.h>" },
    { "ino64_t", "<dirent.h>" },
    { "ino_t", "<sys/types.h>" },
    { "ino_t", "<dirent.h>" },
    { "ino_t", "<sys/stat.h>" },
    { "int8_t", "<sys/types.h>" },
    { "int8_t", "<stdint.h>" },
    { "intptr_t", "<stdint.h>" },
    { "intptr_t", "<unistd.h>" },
    { "key_t", "<sys/types.h>" },
    { "key_t", "<sys/ipc.h>" },
    { "mode_t", "<sys/types.h>" },
    { "mode_t", "<sys/stat.h>" },
    { "mode_t", "<sys/ipc.h>" },
    { "mode_t", "<sys/mman.h>" },
    { "nlink_t", "<sys/types.h>" },
    { "nlink_t", "<sys/stat.h>" },
    { "off64_t", "<sys/types.h>" },
    { "off64_t", "<unistd.h>" },
    { "off_t", "<sys/types.h>" },
    { "off_t", "<unistd.h>" },
    { "off_t", "<sys/stat.h>" },
    { "off_t", "<sys/mman.h>" },
    { "pid_t", "<sys/types.h>" },
    { "pid_t", "<unistd.h>" },
    { "pid_t", "<signal.h>" },
    { "pid_t", "<sys/msg.h>" },
    { "pid_t", "<sys/shm.h>" },
    { "pid_t", "<termios.h>" },
    { "pid_t", "<time.h>" },
    { "pid_t", "<utmpx.h>" },
    { "sigset_t", "<signal.h>" },
    { "sigset_t", "<sys/epoll.h>" },
    { "sigset_t", "<sys/select.h>" },
    { "socklen_t", "<sys/socket.h>" },    // manually mapped from bits/socket.h
    { "socklen_t", "<unistd.h>" },
    { "socklen_t", "<arpa/inet.h>" },
    { "ssize_t", "<sys/types.h>" },
    { "ssize_t", "<unistd.h>" },
    { "ssize_t", "<monetary.h>" },
    { "ssize_t", "<sys/msg.h>" },
    { "suseconds_t", "<sys/types.h>" },
    { "suseconds_t", "<sys/time.h>" },
    { "suseconds_t", "<sys/select.h>" },
    { "u_char", "<sys/types.h>" },
    { "u_char", "<rpc/types.h>" },
    { "uid_t", "<sys/types.h>" },
    { "uid_t", "<unistd.h>" },
    { "uid_t", "<pwd.h>" },
    { "uid_t", "<signal.h>" },
    { "uid_t", "<stropts.h>" },
    { "uid_t", "<sys/ipc.h>" },
    { "uid_t", "<sys/stat.h>" },
    { "useconds_t", "<sys/types.h>" },
    { "useconds_t", "<unistd.h>" },
    // Macros that can be defined in more than one file, don't have
    // the same __foo_defined guard that other types do, so the grep
    // above doesn't discover them.  Until I figure out a better way,
    // I just add them in by hand as I discover them.
    { "EOF", "<stdio.h>" },
    { "EOF", "<libio.h>" },
    // Entries for NULL
    { "NULL", "<stddef.h>" },   // 'canonical' location to get NULL from
    { "NULL", "<clocale>" },
    { "NULL", "<cstddef>" },
    { "NULL", "<cstdio>" },
    { "NULL", "<cstdlib>" },
    { "NULL", "<cstring>" },
    { "NULL", "<ctime>" },
    { "NULL", "<cwchar>" },
    { "NULL", "<locale.h>" },
    { "NULL", "<stdio.h>" },
    { "NULL", "<stdlib.h>" },
    { "NULL", "<string.h>" },
    { "NULL", "<time.h>" },
    { "NULL", "<wchar.h>" },
    // These are c++ symbol maps that handle the forwarding headers
    // that define classes as typedefs.  Because gcc uses typedefs for
    // these, we are tricked into thinking the classes are defined
    // there, rather than just declared there.  This maps each symbol
    // to where it's defined (I had to fix up ios manually, and add in
    // iostream and string which are defined unusually in gcc headers):
    //    ( cd /usr/crosstool/v12/gcc-4.3.1-glibc-2.3.6-grte/x86_64-unknown-linux-gnu/x86_64-unknown-linux-gnu/include/c++/4.3.1; find . -name '*fwd*' | xargs grep -oh 'typedef basic_[^ <]*' | sort -u | sed "s/typedef basic_//" | while read class; do echo -n "$class "; grep -lR "^ *class basic_$class " *; echo | head -n1; done | grep . | perl -lane 'print qq@    { "std::$F[0]", "<$F[1]>" },@;' )
    { "std::filebuf", "<fstream>" },
    { "std::fstream", "<fstream>" },
    { "std::ifstream", "<fstream>" },
    { "std::ios", "<ios>" },
    { "std::iostream", "<iostream>" },
    { "std::istream", "<istream>" },
    { "std::istringstream", "<sstream>" },
    { "std::ofstream", "<fstream>" },
    { "std::ostream", "<ostream>" },
    { "std::ostringstream", "<sstream>" },
    { "std::streambuf", "<streambuf>" },
    { "std::string", "<string>" },
    { "std::stringbuf", "<sstream>" },
    { "std::stringstream", "<sstream>" },
    // Kludge time: almost all STL types take an allocator, but they
    // almost always use the default value.  Usually we detect that
    // and don't try to do IWYU, but sometimes it passes through.
    // For instance, when adding two strings, we end up calling
    //    template<_CharT,_Traits,_Alloc> ... operator+(
    //       basic_string<_CharT,_Traits,_Alloc>, ...)
    // These look like normal template args to us, so we see they're
    // used and declare an iwyu dependency, even though we don't need
    // to #include the traits or alloc type ourselves.  The surest way
    // to deal with this is to just say that everyone provides
    // std::allocator.  We can add more here at need.
    { "std::allocator", "<memory>" },
    { "std::allocator", "<string>" },
    { "std::allocator", "<vector>" },
    { "std::allocator", "<map>" },
    { "std::allocator", "<set>" },
  };

  // We can't make this map transitive, since the keys and values are
  // different: the values are files, but the keys are symbols.
  return MakeIncludeMap(symbol_include_map);
}

IncludePicker::IncludeMap MakeThirdPartyIncludeMap() {
  const char kIcuUtypesHeader[] = "\"third_party/icu/include/unicode/utypes.h\"";

  // It's very common for third-party libraries to just expose one
  // header file.  We use prefix-matching here: the idea is that
  // everything in third_party/foo/... maps to third_party/foo/foo.h, or
  // whatever.  Note this is *not* a transitive include-map, so
  // wherever we map to here, those are the final locations iwyu
  // will return.
  const IncludePicker::IncludeMapEntry third_party_include_map[] = {
    { "third_party/dynamic_annotations/", "\"base/dynamic_annotations.h\"" },
    { "third_party/gmock/include/gmock/", "\"testing/base/public/gmock.h\"" },
    { "third_party/gtest/include/gtest/", "\"testing/base/public/gunit.h\"" },
    { "third_party/python2_4_3/", "<Python.h>" },
    { "third_party/icu/include/unicode/umachine.h", kIcuUtypesHeader },
    { "third_party/icu/include/unicode/uversion.h", kIcuUtypesHeader },
    { "third_party/icu/include/unicode/uconfig.h", kIcuUtypesHeader },
    { "third_party/icu/include/unicode/udraft.h", kIcuUtypesHeader },
    { "third_party/icu/include/unicode/udeprctd.h", kIcuUtypesHeader },
    { "third_party/icu/include/unicode/uobslete.h", kIcuUtypesHeader },
    { "third_party/icu/include/unicode/uintrnal.h", kIcuUtypesHeader },
    { "third_party/icu/include/unicode/usystem.h", kIcuUtypesHeader },
    { "third_party/icu/include/unicode/urename.h", kIcuUtypesHeader },
    { "third_party/icu/include/unicode/platform.h", kIcuUtypesHeader },
    { "third_party/icu/include/unicode/ptypes.h", kIcuUtypesHeader },
    { "third_party/icu/include/unicode/uvernum.h", kIcuUtypesHeader },
    // One day we'll use the system tr1.  But for now it's in third_party
    { "third_party/tr1/tuple_iterate.h", "\"third_party/tr1/tuple\"" },
  };

  return MakeIncludeMap(third_party_include_map);
}

// In linux/gcc, we often see the pattern that there will be
// <asm/foo.h> which just forwards to <asm-ARCH/foo.h>, which will
// actually define the appropriate symbols for a given arch.  In
// these cases, asm/foo.h is the public header, and asm-ARCH is
// private.  This does a private->public mapping for that case.
// Input should be an include-style path but without the quotes.
string NormalizeAsm(string path) {
  if (!StartsWith(path, "asm-"))
    return path;
  StripPast(&path, "/");    // read past "asm-whatever/"
  return "asm/" + path;
}

// Another pattern we see, for system-specific includes, is, e.g.
//    /usr/include/c++/4.2/x86_64-unknown-linux-gnu/bits/c++config.h
// Get rid of the 'x86_64-unknown-linux-gnu' bit.  This should be
// called after initial C++ path parsing, so path is something like
// 'x86_64-unknown-linux-gnu/bits/c++config.h'.
string NormalizeSystemSpecificPath(string path) {
  size_t pos = path.find('/');
  if (pos == string::npos)
    return path;
  // Check that the first directory-path has exactly 3 dashes.
  for (int i = 0; i < 3; ++i) {
    pos = path.rfind('-', pos-1);
    if (pos == string::npos)
      return path;
  }
  if (path.rfind('-', pos-1) != string::npos)
    return path;            // has 4 dashes!
  StripPast(&path, "/");    // read past "x86_64-whatever/"
  return path;
}

}  // namespace

IncludePicker::IncludePicker()
    : cpp_include_multimap_(MakeCppIncludeMap()),
      c_include_multimap_(MakeCIncludeMap()),
      google_include_multimap_(MakeGoogleIncludeMap()),
      third_party_include_multimap_(MakeThirdPartyIncludeMap()),
      dynamic_private_to_public_include_multimap_(),
      symbol_include_multimap_(MakeSymbolIncludeMap()),
      has_called_finalize_added_include_lines_(false) {
}

// Updates dynamic_private_to_public_include_multimap_ based on the include
// seen during this iwyu run.  It includes, for instance, mappings
// from 'project/internal/foo.h' to 'project/public/foo_public.h' in
// google code (Google hides private headers in /internal/, much like
// glibc hides them in /bits/.)  This dynamic mapping is not as good
// as the hard-coded mappings, since it has incomplete information
// (only what is seen during this compile run) and has to do a
// best-effort guess at the mapping.  It's a fallback map when other
// maps come up empty.
void IncludePicker::AddDirectInclude(const string& includer,
                                     const string& include_name_as_typed) {
  assert(!has_called_finalize_added_include_lines_ && "Can't mutate anymore");
  // Note: the includer may be a .cc file, which is unnecessary to add
  // to our map, but harmless.
  const string quoted_includer = GetQuotedIncludeFor(includer);
  if (include_name_as_typed.find("/internal/") != string::npos) {
    if (quoted_includer.find("/internal/") == string::npos) {
      // Add this mapping that crosses the public/private barrier.
      // TODO(csilvers): would be better if key kept the quotes (<> or "")
      // but we take them out to be consistent with the other maps.  That
      // lets us use helper routines such as MakeTransitiveIncludeMap().
      dynamic_private_to_public_include_multimap_.insert(
          make_pair(UnquoteHeader(include_name_as_typed), quoted_includer));
    } else {
      // A private-to-private mapping, which we need to store so we can
      // do transitive mappings.  Like always, we strip the "" and <>
      // from the map-value, to indicate the value is a private header.
      dynamic_private_to_public_include_multimap_.insert(
          make_pair(UnquoteHeader(include_name_as_typed),
                    UnquoteHeader(quoted_includer)));
    }
  }
}

void IncludePicker::AddPrivateToPublicMapping(
    const string& quoted_private_header, const string& public_header) {
  assert(!has_called_finalize_added_include_lines_ && "Can't mutate anymore");
  const string quoted_public_header = GetQuotedIncludeFor(public_header);
  dynamic_private_to_public_include_multimap_.insert(
      make_pair(UnquoteHeader(quoted_private_header), quoted_public_header));
}

// Make dynamic_private_to_public_include_multimap_ transitive, so for an
// #include-path like 'public/foo.h -> private/bar.h -> private/baz.h",
// we correctly map private/baz.h back to public/foo.h.
void IncludePicker::FinalizeAddedIncludes() {
  assert(!has_called_finalize_added_include_lines_ && "Can't call FAI twice");
  dynamic_private_to_public_include_multimap_ =
      MakeTransitiveIncludeMap(dynamic_private_to_public_include_multimap_);
  has_called_finalize_added_include_lines_ = true;
}


void IncludePicker::StripPathPrefixAndGetAssociatedIncludeMap(
    string* path, const IncludeMap** include_map) const {
  *path = NormalizeFilePath(*path);

  if (llvm::sys::path::is_relative(*path)) {  // A relative path
    // Case 1: a local (non-system) include.
    *include_map = StartsWith(*path, "third_party/") ?
        &third_party_include_multimap_ : &google_include_multimap_;
  } else if (StripPast(path, "/c++/") && StripPast(path, "/")) {
    // Case 2: a c++ filename.  It's in a c++/<version#>/include dir.
    // (Or, possibly, c++/<version>/include/x86_64-unknown-linux-gnu/...")
    *path = NormalizeSystemSpecificPath(*path);
    *include_map = &cpp_include_multimap_;
  } else if (StripPast(path, "third_party/stl/gcc") && StripPast(path, "/")) {
    // Case 2b: some c++ files live in third_party/stl/gcc#/
    *path = NormalizeSystemSpecificPath(*path);
    *include_map = &cpp_include_multimap_;
  } else {
    // Case 3: a C filename.  It can be in many locations.
    // TODO(csilvers): get a full list of include dirs from the compiler.
    StripPast(path, "/include/");
    *path = NormalizeAsm(*path);     // Fix up C includes in <asm-ARCH/foo.h>
    *include_map = &c_include_multimap_;
  }
}

vector<string> IncludePicker::GetPublicHeadersForSymbol(
    const string& symbol) const {
  return FindInMultiMap(symbol_include_multimap_, symbol);
}

vector<string> IncludePicker::GetPublicHeadersForPrivateHeader(
    string private_header) const {
  const IncludeMap* include_map;
  StripPathPrefixAndGetAssociatedIncludeMap(&private_header, &include_map);
  vector<string> retval;
  if (include_map == &third_party_include_multimap_) {
    // We use prefix matching instead of the normal exact matching for
    // third-party map.
    for (Each<string, string> it(include_map); !it.AtEnd(); ++it) {
      if (StartsWith(private_header, it->first)) {
        retval.push_back(it->second);
      }
    }
  } else {
    retval = FindInMultiMap(*include_map, private_header);
  }

  if (retval.empty()) {
    // As a last-ditch effort, see if the dynamic map has anything.
    retval = FindInMultiMap(dynamic_private_to_public_include_multimap_,
                            private_header);
  }

  if (retval.empty()) {
    // private_header isn't in include_map, so we just quote and return it.
    const HeaderKind kind =
        IsMapForSystemHeader(include_map) ? kSystemHeader : kUserHeader;
    retval.push_back(QuoteHeader(private_header, kind));
  }

  return retval;
}

bool IncludePicker::PathReexportsInclude(string includer_path,
                                         string includee_path) const {
  const IncludeMap* dummy;
  StripPathPrefixAndGetAssociatedIncludeMap(&includer_path, &dummy);
  const vector<string> candidates =
      GetPublicHeadersForPrivateHeader(includee_path);
  // The candidate-list entries are quoted, but includer_path is not,
  // so we need to unquote to get a fair comparison.
  for (Each<string> it(&candidates); !it.AtEnd(); ++it) {
    if (includer_path == UnquoteHeader(*it))
      return true;
  }
  return false;
}

}  // namespace include_what_you_use
