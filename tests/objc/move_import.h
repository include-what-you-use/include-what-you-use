//===--- move_import.h - test input file for iwyu -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#import "call_instance_method-i1.h"


@interface Foo {
}
- (void)method:(Bar *)var;
@end

/**** IWYU_SUMMARY

tests/objc/move_import.h should add these lines:
@class Bar;

tests/objc/move_import.h should remove these lines:
- #import "call_instance_method-i1.h"  // lines XX-XX

The full include-list for tests/objc/move_import.h:
@class Bar;

***** IWYU_SUMMARY */
