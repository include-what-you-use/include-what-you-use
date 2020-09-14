//===--- protocol_by_class-i1.h - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

@protocol BarProtocol
@optional // optional not to require implementation in other test files
- (void)barMethod;
@end
