//===--- protocol_simple.m - test input file for iwyu ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#import "protocol_simple.h"

void funcFoo(Foo *foo) {}
void funcBar(id<Bar> bar) {}

/**** IWYU_SUMMARY

(tests/objc/protocol_simple.m has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
