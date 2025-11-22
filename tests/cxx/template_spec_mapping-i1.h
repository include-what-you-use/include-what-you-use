//===--- template_spec_mapping-i1.h - test input file for iwyu ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

template <int>
class OtherTpl;

template <typename, int, template <int> typename>
class Tpl {};

template <>
class Tpl<int, 1, OtherTpl> {};

template <typename T, int I, template <int> typename Other>
class Tpl<T*, I, Other> {};

template <typename, typename = char>
class TplWithDefArg {};

template <>
class TplWithDefArg<int*> {};

// Partial specialization for 'char' as the last argument.
template <typename T>
class TplWithDefArg<T> {};

template <long, auto>
class TplWithDeducibleNTTP {};

template <>
class TplWithDeducibleNTTP<1, 1> {};

template <>
class TplWithDeducibleNTTP<1, 1l> {};

template <typename, typename, int>
class TplSpecDistinguishedByIndices {};

template <typename T1, typename T2>
class TplSpecDistinguishedByIndices<T1, T2, 5> {};

template <typename T>
class TplSpecDistinguishedByIndices<T, T, 5> {};

template <typename...>
class TplParamPack1 {};

template <typename T, typename... Args>
class TplParamPack1<int, T, Args...> {};

template <typename... Args>
class TplParamPack1<Args*...> {};

template <typename, typename...>
class TplParamPack2 {};

template <typename T, typename... Args>
class TplParamPack2<int, T, Args...> {};
