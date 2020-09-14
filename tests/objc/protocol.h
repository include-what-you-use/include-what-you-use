//===--- protocol.h - test input file for iwyu ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

@protocol FooProtocol
@optional // optional not to require implementation in other test files
- (void)fooMethod;
@end
