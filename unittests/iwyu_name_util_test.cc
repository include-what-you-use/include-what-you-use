//===--- iwyu_name_util_test.cc - test qualified names --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_ast_util.h"
#include "iwyu_include_picker.h"
#include "iwyu_output.h"

#include <cstdlib>
#include <memory>
#include <string>

#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"
#include "clang/Frontend/ASTUnit.h"
#include "clang/Tooling/Tooling.h"
#include "gtest/gtest.h"

#if defined(__linux__) && GTEST_HAS_DEATH_TEST
#include <sys/resource.h>
#include <unistd.h>
#endif

namespace include_what_you_use {
namespace {

template <typename TestBody>
void RunWithResourceLimits(TestBody test_body) {
#if defined(__linux__) && GTEST_HAS_DEATH_TEST
  // A regression expands billions of template arguments. Confine that failure
  // to a child so it cannot exhaust memory or hang the rest of the test suite.
  ASSERT_EXIT(
      {
        struct rlimit limit;
        if (getrlimit(RLIMIT_AS, &limit) != 0)
          std::_Exit(2);
        constexpr rlim_t kMemoryLimit = 1024 * 1024 * 1024;
        if (limit.rlim_cur == RLIM_INFINITY || limit.rlim_cur > kMemoryLimit)
          limit.rlim_cur = kMemoryLimit;
        if (setrlimit(RLIMIT_AS, &limit) != 0)
          std::_Exit(2);
        limit.rlim_cur = limit.rlim_max = 0;
        if (setrlimit(RLIMIT_CORE, &limit) != 0)
          std::_Exit(2);
        alarm(10);
        test_body();
        std::_Exit(::testing::Test::HasFailure() ? 1 : 0);
      },
      ::testing::ExitedWithCode(0), "");
#else
  GTEST_SKIP()
      << "Deep-name tests require Linux resource limits and death tests";
#endif
}

const clang::VarDecl* FindVariable(const clang::ASTUnit& ast) {
  for (const clang::Decl* decl :
       ast.getASTContext().getTranslationUnitDecl()->decls()) {
    if (const auto* variable = llvm::dyn_cast<clang::VarDecl>(decl))
      return variable;
  }
  return nullptr;
}

std::unique_ptr<clang::ASTUnit> BuildDeepMember() {
  std::string code =
      "template<class A, class B> struct Pair {};"
      "template<class T> using Dup = Pair<T, T>;"
      "template<class T> struct Outer { int member; };"
      "Outer<";
  // Parsing shares this type graph, but spelling the member's canonical
  // qualifier would expand more than four billion copies of int. Exercise
  // name handling directly rather than unrelated recursive IWYU traversal.
  for (int i = 0; i < 32; ++i)
    code += "Dup<";
  code += "int";
  code.append(32, '>');
  code += "> object;";
  return clang::tooling::buildASTFromCodeWithArgs(code, {"-std=c++17"});
}

TEST(NameUtil, UnneededMemberNameDoesNotExpandTemplateArguments) {
  RunWithResourceLimits([] {
    std::unique_ptr<clang::ASTUnit> ast = BuildDeepMember();
    ASSERT_TRUE(ast);
    const clang::VarDecl* variable = FindVariable(*ast);
    ASSERT_NE(nullptr, variable);
    const clang::CXXRecordDecl* record =
        variable->getType()->getAsCXXRecordDecl();
    ASSERT_NE(nullptr, record);
    ASSERT_FALSE(record->field_empty());
    const clang::FieldDecl* member = *record->field_begin();

    OneUse use(member, {}, {}, UseKind::Full, UF_None, nullptr);
    EXPECT_EQ("Outer<>::member", use.short_symbol_name());
    use.set_ignore_use();
    EXPECT_TRUE(use.ignore_use());
  });
}

TEST(NameUtil, MemberQualifierCannotMatchTemplateArgumentFreeName) {
  RunWithResourceLimits([] {
    std::unique_ptr<clang::ASTUnit> ast = BuildDeepMember();
    ASSERT_TRUE(ast);
    const clang::VarDecl* variable = FindVariable(*ast);
    ASSERT_NE(nullptr, variable);
    const clang::CXXRecordDecl* record =
        variable->getType()->getAsCXXRecordDecl();
    ASSERT_NE(nullptr, record);
    ASSERT_FALSE(record->field_empty());
    EXPECT_FALSE(
        HasWrittenQualifiedNameIn(*record->field_begin(), {"std::member"}));
  });
}

TEST(NameUtil, FixedNameRequiresExactQualification) {
  std::unique_ptr<clang::ASTUnit> ast = clang::tooling::buildASTFromCode(
      "namespace example { struct Named {}; } example::Named object;");
  ASSERT_TRUE(ast);
  const clang::VarDecl* variable = FindVariable(*ast);
  ASSERT_NE(nullptr, variable);
  const clang::CXXRecordDecl* record =
      variable->getType()->getAsCXXRecordDecl();
  ASSERT_NE(nullptr, record);
  EXPECT_TRUE(HasWrittenQualifiedNameIn(record, {"example::Named"}));
  EXPECT_FALSE(HasWrittenQualifiedNameIn(record, {"other::Named"}));
  EXPECT_FALSE(HasWrittenQualifiedNameIn(record, {"example::Other"}));
  EXPECT_DEATH(HasWrittenQualifiedNameIn(record, {"Named"}), "qualified");
}

TEST(NameUtil, ExplicitSpecializationCannotMatchTemplateArgumentFreeName) {
  std::unique_ptr<clang::ASTUnit> ast = clang::tooling::buildASTFromCode(
      "namespace example { template<class> struct Named;"
      "template<> struct Named<int> {}; } example::Named<int> object;");
  ASSERT_TRUE(ast);
  const clang::VarDecl* variable = FindVariable(*ast);
  ASSERT_NE(nullptr, variable);
  EXPECT_FALSE(HasWrittenQualifiedNameIn(
      variable->getType()->getAsCXXRecordDecl(), {"example::Named"}));
}

}  // namespace
}  // namespace include_what_you_use
