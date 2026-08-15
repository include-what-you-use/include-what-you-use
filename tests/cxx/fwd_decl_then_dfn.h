//===--- fwd_decl_then_dfn.h - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// fwd_decl_then_dfn.cc content has been duplicated here to test it with
// a different provision policy (header files may provide stuff not strictly
// required to compile them, whereas source files should not).

template <class T>
struct HeaderFoo;
template <class T>
struct HeaderSubFoo : public HeaderFoo<T> {};

template <class T>
struct HeaderBar;
template <class T>
struct HeaderSubBar : public HeaderFoo<typename HeaderBar<T>::type> {};

template <class T>
struct HeaderBaz;
typedef HeaderBaz<int> HeaderBazTypedef;

struct HeaderBang;
typedef HeaderBang HeaderBangTypedef;

// Now come the definitions, way at the end.

template <class T>
struct HeaderFoo {};
template <class T>
struct HeaderBar {
  typedef const T type;
};
template <class T>
struct HeaderBaz {};
struct HeaderBang {};

/**** IWYU_SUMMARY

(tests/cxx/fwd_decl_then_dfn.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
