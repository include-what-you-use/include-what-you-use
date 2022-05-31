//===--- protocol_by_class.m - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Wno-objc-root-class

#import "protocol_by_class.h"

@implementation Foo
@end

/**** IWYU_SUMMARY

(tests/objc/protocol_by_class.m has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
