//===--- fwd_decl_with_attribute.cc - test input file for iwyu ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests how attributes affect forward declarations.

// Try a type that only needs a forward-declare because of attributes.
struct __attribute__((aligned (16))) AttrStruct;
struct AttrStruct* attrstruct_ptr = 0;

// Test that forward-declare with inherited attributes is removed.
struct __attribute__((aligned (16))) InheritedAttrStruct {};
struct InheritedAttrStruct;
struct InheritedAttrStruct* inherited_attrstruct_ptr = 0;

/**** IWYU_SUMMARY

tests/fwd_decl_with_attribute.cc should add these lines:

tests/fwd_decl_with_attribute.cc should remove these lines:
- struct InheritedAttrStruct;  // lines XX-XX

The full include-list for tests/fwd_decl_with_attribute.cc:
struct AttrStruct;  // lines XX-XX

***** IWYU_SUMMARY */
