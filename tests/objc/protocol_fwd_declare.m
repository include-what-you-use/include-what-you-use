//===--- protocol_fwd_declare.m - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Wno-objc-root-class

#import "protocol_fwd_declare.h"

@implementation Foo
@end

/**** IWYU_SUMMARY

(tests/objc/protocol_fwd_declare.m has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
