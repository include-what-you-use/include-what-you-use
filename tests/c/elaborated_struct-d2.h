//===--- elaborated_struct-d2.h - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

typedef struct Struct Typedef;

// Similar to LocalStructAlias.
typedef struct HeaderStruct HeaderStructAlias;
struct HeaderStruct {
  int x;
  HeaderStructAlias* next;
};

/**** IWYU_SUMMARY

(tests/c/elaborated_struct-d2.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
