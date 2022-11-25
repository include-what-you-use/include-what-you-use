//===--- dependent_tpl_crash.cc - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that we don't trigger an assertion failure for dependent template
// aliases. We avoid reporting uses of template names without an underlying
// template decl to ensure we don't hit this.

template <int value, typename Type>
struct ClassValueType {};

template <int value>
struct ClassValue {
  // This would fail with:
  //   Assertion failed: Val && "isa<> used on a null pointer"
  // because a dependent template alias does not have an underlying template
  // decl.
  template <typename Type>
  using ClassType = ClassValueType<value, Type>;
};

template <template <typename> typename TemplateClassType>
struct FinalType {
  static void run() {
    TemplateClassType<int> instance{};
  }
};

template <int value>
void test() {
  using Type = FinalType<ClassValue<value>::template ClassType>;
  Type::run();
}

int main() {
  test<0>();
}

/**** IWYU_SUMMARY

(tests/cxx/dependent_tpl_crash.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
