//===--- protocol_by_category.h - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#import "imported_class.h"
#import "protocol.h"

@interface ImportedClass(FooCategory) <FooProtocol>
@end

/**** IWYU_SUMMARY

(tests/objc/protocol_by_category.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
