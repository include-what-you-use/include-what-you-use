//===--- protocol_fwd_declare_property.m - test input file for iwyu -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Wno-objc-root-class

#import "protocol_fwd_declare_property.h"

@implementation Foo

// We are lying here, nobody implements these properties. Don't implement
// properties to keep test small and devoted to properties in .h file.
@dynamic idProperty;
@dynamic barProperty;
@dynamic classProperty;

@end

/**** IWYU_SUMMARY

(tests/objc/protocol_fwd_declare_property.m has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
