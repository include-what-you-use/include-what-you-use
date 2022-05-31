//===--- call_class_method.m - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Wno-objc-root-class

#import "call_class_method.h"
#import "call_class_method-i1.h"

@implementation Foo

- (void)method {
  [Bar classMethod];
}

@end

/**** IWYU_SUMMARY

(tests/objc/call_class_method.m has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
