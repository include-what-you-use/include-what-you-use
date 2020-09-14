//===--- unused_fwd_decl_and_import.h - test input file for iwyu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#import "imported_class.h"

@class ImportedClass;

@interface C {
}
@end

/**** IWYU_SUMMARY

tests/objc/unused_fwd_decl_and_import.h should add these lines:

tests/objc/unused_fwd_decl_and_import.h should remove these lines:
- #import "imported_class.h"  // lines XX-XX
- @class ImportedClass;  // lines XX-XX

The full include-list for tests/objc/unused_fwd_decl_and_import.h:

***** IWYU_SUMMARY */
