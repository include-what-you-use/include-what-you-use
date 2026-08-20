//===--- anonymous_struct.h - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

typedef union {
  int tu;
  int ty;
} header_typedef_union;

typedef struct {
  int ta;
  int tb;
} header_typedef_struct;

typedef struct header_typedef_struct_with_label {
  int td1;
  int td2;
} header_typedef_struct_with_label;

typedef enum { TA_Header, TB_Header } header_typedef_enum;

/**** IWYU_SUMMARY

(tests/cxx/anonymous_struct.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
