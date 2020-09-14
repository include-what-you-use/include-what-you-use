//===--- protocol_by_protocol.m - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Wno-objc-root-class

#import "protocol.h"

//Note: it makes little sense to declare protocol not in .h file, for details
// see comment for BazClass
@protocol BazProtocol <FooProtocol>
@optional
- (void)bazMethod;
@end

//vsapsai: this class is unrelated to tested protocol and introduced only to
// have .m file and to run test automatically for this file.
@interface BazClass {
}
@end

@implementation BazClass
@end

/**** IWYU_SUMMARY

(tests/objc/protocol_by_protocol.m has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
