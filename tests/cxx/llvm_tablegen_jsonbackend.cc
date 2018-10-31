//===--- llvm_tablegen_jsonbackend.cc - test input file for iwyu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that IWYU doesn't crash when building
// llvm/lib/TableGen/JSONBackend.cpp

template <int a>
struct b {
  static constexpr int c = a;
};
template <bool a>
using d = b<a>;
template <bool, typename>
struct aa;
template <typename...>
struct e;
template <typename f, typename g>
struct e<f, g> : aa<f::c, g>::h {};
template <typename i>
i ab();
struct j {
  b<true> k;
};
template <typename>
struct l : j {
  typedef decltype(k) h;
};
template <typename i>
struct m : e<d<!bool()>, l<i>> {};
template <typename i>
struct n : m<i>::h {};
struct o {
  static b<false> p(...);
};
template <typename i>
struct q : o {
  decltype(p(ab<i>())) h;
};
template <typename i>
struct r : e<n<i>, q<i>> {};
template <int>
struct s;
template <bool, typename ac>
struct aa {
  typedef ac h;
};
class G;
struct H {
  template <typename t = G, typename u = int,
            typename s<e<r<t>, u>::c>::h = true>
  H();
};
class G {
  G(G &);
};
struct I {
  H ar;
};

/**** IWYU_SUMMARY

tests/cxx/llvm_tablegen_jsonbackend.cc should add these lines:

tests/cxx/llvm_tablegen_jsonbackend.cc should remove these lines:
- template <int> struct s;  // lines XX-XX+1

The full include-list for tests/cxx/llvm_tablegen_jsonbackend.cc:
class G;  // lines XX-XX
template <bool, typename > struct aa;  // lines XX-XX+1
template <typename ...> struct e;  // lines XX-XX+1

***** IWYU_SUMMARY */
