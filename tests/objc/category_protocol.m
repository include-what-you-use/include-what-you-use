//===--- category_protocol.m - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#import "category_protocol.h"
#import "category_protocol-1.h"
#import "category_protocol-2.h"

@interface MyClass(Foo) <Foo>
@end

@implementation MyClass(Foo)
@end

void funcBar(id<Foo, Bar> x) {}

/**** IWYU_SUMMARY

tests/objc/category_protocol.m should add these lines:

tests/objc/category_protocol.m should remove these lines:
- #import "category_protocol-2.h"  // lines XX-XX

The full include-list for tests/objc/category_protocol.m:
#import "category_protocol.h"
#import "category_protocol-1.h"  // for Foo

***** IWYU_SUMMARY */
