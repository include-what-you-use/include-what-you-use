//===--- template_spec_mapping.cc - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --mapping_file=tests/cxx/template_spec_mapping.imp -I .

#include "tests/cxx/template_spec_mapping-d1.h"

// TODO: OtherTpl is forward-declarable.
// IWYU: OtherTpl is...*-i1.h
// IWYU: Tpl is...*-i2.h
Tpl<char, 2, OtherTpl> t1;
// IWYU: OtherTpl is...*-i1.h
// IWYU: Tpl<int, 1, OtherTpl> is...*-i3.h
Tpl<int, 1, OtherTpl> t2;
// IWYU: OtherTpl is...*-i1.h
// IWYU: Tpl<:0 \*, :1, :2> is...*-i4.h
Tpl<int*, 1, OtherTpl> t3;

// IWYU: TplWithDefArg is...*-i2.h
TplWithDefArg<int, int> twda1;
// TODO: reporting the primary template due to the presence of a default
// argument is redundant here.
// IWYU: TplWithDefArg is...*-i2.h
// IWYU: TplWithDefArg<int \*, char> is...*-i3.h
TplWithDefArg<int*> twda3;
// IWYU: TplWithDefArg is...*-i2.h
// IWYU: TplWithDefArg<:0, char> is...*-i4.h
TplWithDefArg<int> twda2;

// IWYU: TplWithDeducibleNTTP is...*-i2.h
TplWithDeducibleNTTP<2, 3> twdnttp1;
// IWYU: TplWithDeducibleNTTP<1, 1> is...*-i3.h
TplWithDeducibleNTTP<1, 1> twdnttp2;
// IWYU: TplWithDeducibleNTTP<1, 1L> is...*-i4.h
TplWithDeducibleNTTP<1, 1l> twdnttp3;

// IWYU: TplSpecDistinguishedByIndices is...*-i2.h
TplSpecDistinguishedByIndices<int, char, 7> tsdbi1;
// IWYU: TplSpecDistinguishedByIndices<:0, :1, 5> is...*-i3.h
TplSpecDistinguishedByIndices<int, char, 5> tsdbi2;
// IWYU: TplSpecDistinguishedByIndices<:0, :0, 5> is...*-i4.h
TplSpecDistinguishedByIndices<int, int, 5> tsdbi3;

// IWYU: TplParamPack1 is...*-i2.h
TplParamPack1<long, short> tpp11;
// IWYU: TplParamPack1<int, :0, :1...> is...*-i3.h
TplParamPack1<int, char> tpp12;
// IWYU: TplParamPack1<int, :0, :1...> is...*-i3.h
TplParamPack1<int, char, float> tpp13;
// IWYU: TplParamPack1<int, :0, :1...> is...*-i3.h
TplParamPack1<int, char, float, double> tpp14;
// IWYU: TplParamPack1<:0 \*...> is...*-i4.h
TplParamPack1<int*, char*> tpp15;

// IWYU: TplParamPack2 is...*-i2.h
TplParamPack2<double, char, float> tpp21;
// IWYU: TplParamPack2<int, :0, :1...> is...*-i3.h
TplParamPack2<int, char, float> tpp22;

/**** IWYU_SUMMARY

tests/cxx/template_spec_mapping.cc should add these lines:
#include "tests/cxx/template_spec_mapping-i1.h"
#include "tests/cxx/template_spec_mapping-i2.h"
#include "tests/cxx/template_spec_mapping-i3.h"
#include "tests/cxx/template_spec_mapping-i4.h"

tests/cxx/template_spec_mapping.cc should remove these lines:
- #include "tests/cxx/template_spec_mapping-d1.h"  // lines XX-XX

The full include-list for tests/cxx/template_spec_mapping.cc:
#include "tests/cxx/template_spec_mapping-i1.h"  // for OtherTpl
#include "tests/cxx/template_spec_mapping-i2.h"  // for Tpl, TplParamPack1, TplParamPack2, TplSpecDistinguishedByIndices, TplWithDeducibleNTTP, TplWithDefArg
#include "tests/cxx/template_spec_mapping-i3.h"  // for Tpl, TplParamPack1, TplParamPack2, TplSpecDistinguishedByIndices, TplWithDeducibleNTTP, TplWithDefArg
#include "tests/cxx/template_spec_mapping-i4.h"  // for Tpl, TplParamPack1, TplSpecDistinguishedByIndices, TplWithDeducibleNTTP, TplWithDefArg

***** IWYU_SUMMARY */
