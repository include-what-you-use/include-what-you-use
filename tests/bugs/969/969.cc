//===--- 969.cc - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#define define_my_var           \
struct my_struct {              \
        int my_member;          \
};                              \
struct my_struct my_var;

define_my_var

/**** IWYU_SUMMARY

(tests/bugs/969/969.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
