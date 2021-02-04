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

@interface Tsar
@end

@implementation Tsar
@end

/**** IWYU_SUMMARY

(tests/objc/module.m has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
