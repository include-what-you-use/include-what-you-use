//===--- call_instance_method_simple.h - test input file for iwyu ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

@class Bar;

void foo(Bar *b);

/**** IWYU_SUMMARY

(tests/objc/call_instance_method_simple.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
