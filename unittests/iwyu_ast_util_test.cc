//===--- iwyu_ast_util_test.cc - test structural type helpers ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_ast_util.h"

#include <cstddef>
#include <cstdlib>
#include <memory>
#include <set>
#include <string>

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Type.h"
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
  // Without deduplication the small shared graph below has billions of paths.
  // A regression must fail this child, not hang or exhaust the CI runner.
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
      << "Deep-type tests require Linux resource limits and death tests";
#endif
}

TEST(TypeEnumerator, RetainsDifferentSpellingsOfSharedType) {
  std::unique_ptr<clang::ASTUnit> ast =
      clang::tooling::buildASTFromCodeWithArgs(
          "template<class A, class B> struct Pair {};"
          "struct Payload {};"
          "using LocalPayload = Payload;"
          "using ProvidedPayload = Payload;"
          "Pair<LocalPayload, ProvidedPayload> object;",
          {"-std=c++17"});
  ASSERT_TRUE(ast);
  const clang::VarDecl* variable = nullptr;
  for (const clang::Decl* decl :
       ast->getASTContext().getTranslationUnitDecl()->decls()) {
    if (const auto* candidate = llvm::dyn_cast<clang::VarDecl>(decl))
      variable = candidate;
  }
  ASSERT_NE(nullptr, variable);
  const auto* pair = llvm::dyn_cast<clang::TemplateSpecializationType>(
      Desugar(variable->getType().getTypePtr()));
  ASSERT_NE(nullptr, pair);
  const clang::Type* local =
      Desugar(pair->template_arguments()[0].getAsType().getTypePtr());
  const clang::Type* provided =
      Desugar(pair->template_arguments()[1].getAsType().getTypePtr());
  ASSERT_NE(local, provided);
  ASSERT_EQ(GetCanonicalType(local), GetCanonicalType(provided));

  // Both spellings matter to callers even though their recursive descent
  // reaches the same Payload node. Check the set before resugaring or
  // reporting.
  const std::set<const clang::Type*> expected = {pair, local, provided};
  EXPECT_EQ(expected, GetComponentsOfType(pair));
}

void CheckSharedAliasGraph(bool canonical) {
  constexpr int kDepth = 32;
  std::string code =
      "template<class A, class B> struct Pair {}; using D0 = int; D0* p0;";
  for (int i = 1; i <= kDepth; ++i) {
    const std::string previous = "D" + std::to_string(i - 1);
    code += "using D" + std::to_string(i) + " = Pair<" + previous + "," +
            previous + ">; D" + std::to_string(i) + "* p" + std::to_string(i) +
            ";";
  }
  std::unique_ptr<clang::ASTUnit> ast =
      clang::tooling::buildASTFromCodeWithArgs(code, {"-std=c++17"});
  ASSERT_TRUE(ast);
  std::set<const clang::Type*> expected;
  const clang::Type* root = nullptr;
  for (const clang::Decl* decl :
       ast->getASTContext().getTranslationUnitDecl()->decls()) {
    if (const auto* variable = llvm::dyn_cast<clang::VarDecl>(decl)) {
      root = variable->getType()->getPointeeType().getTypePtr();
      ASSERT_NE(nullptr, root);
      expected.insert(canonical ? GetCanonicalType(root) : root);
    }
  }
  ASSERT_NE(nullptr, root);
  ASSERT_EQ(static_cast<std::size_t>(kDepth + 1), expected.size());

  // Source size and the expected set grow linearly, but each Pair exposes the
  // preceding alias twice. Invoke only the structural helper so unrelated IWYU
  // reporting, provider analysis and qualified-name printing cannot dominate.
  EXPECT_EQ(expected, canonical ? GetCanonicalComponentsOfType(root)
                                : GetComponentsOfType(root));
}

TEST(TypeEnumerator, SugaredEnumerationDoesNotExpandSharedAliasPaths) {
  RunWithResourceLimits([] { CheckSharedAliasGraph(false); });
}

TEST(TypeEnumerator, CanonicalEnumerationDoesNotExpandSharedAliasPaths) {
  RunWithResourceLimits([] { CheckSharedAliasGraph(true); });
}

}  // namespace
}  // namespace include_what_you_use
