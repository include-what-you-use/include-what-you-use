//===--- protocol_by_class.h - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#import "protocol.h"
#import "protocol_by_class-i1.h"

@interface Foo <FooProtocol, BarProtocol> {
}
@end

/**** IWYU_SUMMARY

(tests/objc/protocol_by_class.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
