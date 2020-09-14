//===--- unused_fwd_decl_simple.m - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

@class Unused;
@class Used;

void foo(Used *used) {}

/**** IWYU_SUMMARY

tests/objc/unused_fwd_decl_simple.m should add these lines:

tests/objc/unused_fwd_decl_simple.m should remove these lines:
- @class Unused;  // lines XX-XX

The full include-list for tests/objc/unused_fwd_decl_simple.m:
@class Used;  // lines XX-XX

***** IWYU_SUMMARY */
