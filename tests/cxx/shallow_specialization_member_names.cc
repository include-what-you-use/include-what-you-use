//===--- shallow_specialization_member_names.cc - test input ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Six levels retain shared substituted types without making the independent
// traversal oracle or the retraversal self-check expand millions of paths.

#include "tests/cxx/deep_specialization_member_names-defs.h"

Outer<Dup<Dup<Dup<Dup<Dup<Dup<Payload>>>>>>>::Member<int> member;
Outer<Dup<Dup<Dup<Dup<Dup<Dup<Payload>>>>>>>::map<int> map;
Outer<Dup<Dup<Dup<Dup<Dup<Dup<Payload>>>>>>>::ostream stream = 0;

/**** IWYU_SUMMARY

(tests/cxx/shallow_specialization_member_names.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
