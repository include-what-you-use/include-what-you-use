//===--- fwd_decl_mapping-i1.h - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

class OnlyFwdDeclUse {};

class FwdDeclAndFullUse {};

class FwdDeclInTwoHeaders1 {};

class FwdDeclInI6 {};

class HasIncludeMapping {};

class HasMappingForFullUse {};

class FwdDeclInTwoHeaders2 {};

class MappedToD1 {};

enum class EnumClass1 { A, B, C };

enum class EnumClass2 { A, B, C };
