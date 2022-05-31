//===--- protocol_simple.h - test input file for iwyu ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

@class Foo;
@protocol Bar;

void funcFoo(Foo *foo);
void funcFoo2(Foo *foo);
void funcBar(id<Bar> bar);

/**** IWYU_SUMMARY

(tests/objc/protocol_simple.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
