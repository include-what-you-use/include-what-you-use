//===--- imported_class.m - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Wno-objc-root-class

#import "imported_class.h"

@implementation ImportedClass
@end

/**** IWYU_SUMMARY

(tests/objc/imported_class.m has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
