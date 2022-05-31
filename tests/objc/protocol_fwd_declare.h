//===--- protocol_fwd_declare.h - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#import "protocol.h"
@class ImportedClass;
@protocol BarProtocol;
@protocol ClassProtocol;

@interface Foo {
 @private
  id <FooProtocol> fooVar;
  ImportedClass <BarProtocol> *barVar;
  Class <ClassProtocol> classVar;
}
@end

/**** IWYU_SUMMARY

tests/objc/protocol_fwd_declare.h should add these lines:
@protocol FooProtocol;

tests/objc/protocol_fwd_declare.h should remove these lines:
- #import "protocol.h"  // lines XX-XX

The full include-list for tests/objc/protocol_fwd_declare.h:
@class ImportedClass;  // lines XX-XX
@protocol BarProtocol;  // lines XX-XX
@protocol ClassProtocol;  // lines XX-XX
@protocol FooProtocol;

***** IWYU_SUMMARY */
