//===--- protocol_fwd_declare_property.h - test input file for iwyu -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

@class Bar;
@protocol BarProtocol;
@protocol ClassProtocol;
@protocol Delegate;
@protocol FooProtocol;


@interface Foo {
}
@property (assign, nonatomic) id <Delegate, FooProtocol> idProperty;
@property (retain, nonatomic) Bar <BarProtocol> *barProperty;
@property (assign, nonatomic) Class <ClassProtocol> classProperty;
@end

/**** IWYU_SUMMARY

(tests/objc/protocol_fwd_declare_property.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
