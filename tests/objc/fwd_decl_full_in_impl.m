//===--- fwd_decl_full_in_impl.m - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Wno-objc-root-class

#import "fwd_decl_full_in_impl.h"
#import "imported_class.h"
#import "protocol.h"

@implementation C
@end

/**** IWYU_SUMMARY

tests/objc/fwd_decl_full_in_impl.m should add these lines:

tests/objc/fwd_decl_full_in_impl.m should remove these lines:
- #import "imported_class.h"  // lines XX-XX
- #import "protocol.h"  // lines XX-XX

The full include-list for tests/objc/fwd_decl_full_in_impl.m:
#import "fwd_decl_full_in_impl.h"

***** IWYU_SUMMARY */
