//===--- 1707.h - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include <type_traits>
#include <variant>
#include <vector>

struct Foo;

// template magic from https://stackoverflow.com/a/45896101
template <class T, class U>
struct is_one_of;
template <class T, class... Ts>
struct is_one_of<T, std::variant<Ts...>>
    : std::bool_constant<(std::is_same_v<T, Ts> || ...)> {};
template <class T, class V>
using is_variant_type = is_one_of<T, V>;

struct Foo {
  struct Foo_Inner;
  using impl_t = std::variant<Foo_Inner>;

  Foo() = default;

  template <class U, std::enable_if_t<is_variant_type<U, impl_t>{}, int> = 0>
  explicit Foo(U u) {
  }

  struct Foo_Inner {};

  impl_t data;
};

class Bar {
  void stuff(Foo value) {
    my_vec.push_back(value);
  }

  std::vector<Foo> my_vec;
};

/**** IWYU_SUMMARY

(tests/bugs/1707/1707.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
