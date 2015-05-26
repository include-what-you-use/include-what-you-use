//===--- anonymous_struct.cc - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that anonymous structs, unions, and enums don't cause iwyu problems.

union {
  int x;
  int y;
} my_union;

struct {
  int a;
  int b;
} my_struct;

enum { A, B } my_enum;

// This isn't an anonymous struct, but I'm sneaking it in anyway,
// since it's used in the same context we often see anonymous structs
// used.

struct DefinePlusVarStruct {
  int d1;
  int d2;
} my_define_plus_struct;

// Inside a struct

struct Foo {
  union {
    int sx;
    int sy;
  } struct_union;
  struct {
    int sa;
    int sb;
  } struct_struct;
  struct DefinePlusVarStruct {
    int sd1;
    int sd2;
  } struct_define_plus_struct;
  enum { SA, SB } struct_enum;
};

// Part of a typedef (very common in C!)

typedef union {
  int tu;
  int ty;
} typedef_union;

typedef struct {
  int ta;
  int tb;
} typedef_struct;

typedef struct typedef_struct_with_label {
  int td1;
  int td2;
} typedef_struct_with_label;

typedef enum { TA, TB } typedef_enum;

typedef_union tu;
typedef_struct ts;
typedef_struct_with_label tswl;
typedef_enum te;


/**** IWYU_SUMMARY

(tests/cxx/anonymous_struct.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
