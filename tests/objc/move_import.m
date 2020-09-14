//===--- move_import.m - test input file for iwyu -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Wno-objc-root-class

#import "move_import.h"

@implementation Foo

- (void)method:(Bar *)var {
  [var instanceMethod];
}

@end

/**** IWYU_SUMMARY

tests/objc/move_import.m should add these lines:
#import "call_instance_method-i1.h"

tests/objc/move_import.m should remove these lines:

The full include-list for tests/objc/move_import.m:
#import "move_import.h"
#import "call_instance_method-i1.h"  // for Bar, Bar::instanceMethod

***** IWYU_SUMMARY */
