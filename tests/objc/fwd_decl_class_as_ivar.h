//===--- fwd_decl_class_as_ivar.h - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#import "imported_class.h"

@interface C {
 @private
  ImportedClass *var;
}
@end

/**** IWYU_SUMMARY

tests/objc/fwd_decl_class_as_ivar.h should add these lines:
@class ImportedClass;

tests/objc/fwd_decl_class_as_ivar.h should remove these lines:
- #import "imported_class.h"  // lines XX-XX

The full include-list for tests/objc/fwd_decl_class_as_ivar.h:
@class ImportedClass;

***** IWYU_SUMMARY */
