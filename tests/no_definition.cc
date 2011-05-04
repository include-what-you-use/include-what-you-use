//===--- no_definition.cc - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests we keep at least one forward-decl even when there's no definition.

class Foo;
Foo* foo;

template <typename T> class Bar;
Bar<int>* bar;

// The tricky case -- because of the default parameter, we want to
// replace the fwd-decl with a full use, but there *is* no full use.
template <typename T, typename U = int> class Baz;
Baz<int>* baz;



/**** IWYU_SUMMARY

(tests/no_definition.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
