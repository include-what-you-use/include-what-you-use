//===--- call_instance_method_simple.m - test input file for iwyu ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "call_instance_method_simple.h"
#include "call_instance_method_simple-i1.h"

void foo(Bar *b) {
    [b instanceMethod];
}

/**** IWYU_SUMMARY

(tests/objc/call_instance_method_simple.m has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
