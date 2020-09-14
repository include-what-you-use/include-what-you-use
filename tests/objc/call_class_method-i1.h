//===--- call_class_method-i1.h - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

@interface Bar {
}
+ (void)classMethod;
@end

// @implementation in .h-file is bad practice, do it to avoid more helper files
@implementation Bar
+ (void)classMethod {}
@end
