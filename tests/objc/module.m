//===--- protocol_simple.m - test input file for iwyu ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -fmodules -F tests/objc/frameworks -Wno-objc-root-class

#import <Foo/Foo.h>
#import "module.h"
@import Foo;
@import Foo;
#import <stdio.h>
#import <stdio.h>

@interface Tsar: Foo
@end

@implementation Tsar
@end

void test() {
  printf("hello");
}

/**** iwyu_summary

(tests/objc/module.m has correct #includes/fwd-decls)

***** iwyu_summary */
