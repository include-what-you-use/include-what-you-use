//===--- static_data_member-i2.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

template <typename T>
struct Tpl {
  static int i;
  static T* fwd_decl_use_in_type;
  static T full_use_in_type;
  static int full_use_in_init;
  template <typename U>
  static U fully_using_both;
};

class IndirectClass;
using TplNonProvidingIC = Tpl<IndirectClass>;

template <typename>
struct PartiallySpecializedTpl;

template <typename T>
struct PartiallySpecializedTpl<T*> {
  static int i;
};

template <typename>
struct TplWithMapping {
  static int i;
};

template <typename>
struct PartiallySpecializedTplWithMapping;

template <typename T>
struct PartiallySpecializedTplWithMapping<T*> {
  static int i;
};
