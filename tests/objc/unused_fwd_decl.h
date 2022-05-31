//===--- unused_fwd_decl.h - test input file for iwyu ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

@class ImportedClass;

@interface C {
}
@end

/**** IWYU_SUMMARY

tests/objc/unused_fwd_decl.h should add these lines:

tests/objc/unused_fwd_decl.h should remove these lines:
- @class ImportedClass;  // lines XX-XX

The full include-list for tests/objc/unused_fwd_decl.h:

***** IWYU_SUMMARY */
