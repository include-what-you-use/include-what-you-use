//===--- category_protocol.h - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

@protocol Foo;
@protocol Bar;

@interface MyClass
@end

/**** IWYU_SUMMARY

tests/objc/category_protocol.h should add these lines:

tests/objc/category_protocol.h should remove these lines:
- @protocol Bar;  // lines XX-XX
- @protocol Foo;  // lines XX-XX

The full include-list for tests/objc/category_protocol.h:

***** IWYU_SUMMARY */
