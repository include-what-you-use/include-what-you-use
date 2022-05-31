//===--- fwd_decl_class_as_property.m - test input file for iwyu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Wno-objc-root-class

#import "fwd_decl_class_as_property.h"

@implementation Foo

// We are lying here, nobody implements these properties. Don't implement
// properties to keep test small and devoted to properties in .h file.
@dynamic fooProperty;
@dynamic idProperty;
@dynamic intProperty;

@end

/**** IWYU_SUMMARY

(tests/objc/fwd_decl_class_as_property.m has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
