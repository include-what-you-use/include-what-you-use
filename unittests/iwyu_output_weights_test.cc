//===--- iwyu_output_weights_test.cc - weighted output tests --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_output.h"

#include <cstdint>
#include <limits>

#include "gtest/gtest.h"

namespace include_what_you_use {

namespace {

constexpr uint64_t kMaxWeight = std::numeric_limits<uint64_t>::max();

TEST(UseWeights, AdditionPreservesRepresentableCounts) {
  EXPECT_EQ(0u, AddUseWeights(0, 0));
  EXPECT_EQ(42u, AddUseWeights(17, 25));
  EXPECT_EQ(kMaxWeight, AddUseWeights(kMaxWeight - 1, 1));
  EXPECT_EQ(kMaxWeight, AddUseWeights(0, kMaxWeight));
}

TEST(UseWeights, AdditionSaturatesInsteadOfWrapping) {
  EXPECT_EQ(kMaxWeight, AddUseWeights(kMaxWeight, 1));
  EXPECT_EQ(kMaxWeight, AddUseWeights(1, kMaxWeight));
  EXPECT_EQ(kMaxWeight, AddUseWeights(kMaxWeight, kMaxWeight));
}

TEST(UseWeights, MultiplicationPreservesRepresentableCounts) {
  EXPECT_EQ(0u, MultiplyUseWeights(0, kMaxWeight));
  EXPECT_EQ(0u, MultiplyUseWeights(kMaxWeight, 0));
  EXPECT_EQ(42u, MultiplyUseWeights(6, 7));
  EXPECT_EQ(kMaxWeight, MultiplyUseWeights(kMaxWeight, 1));
  EXPECT_EQ(kMaxWeight - 1, MultiplyUseWeights(kMaxWeight / 2, 2));
}

TEST(UseWeights, MultiplicationSaturatesInsteadOfWrapping) {
  EXPECT_EQ(kMaxWeight, MultiplyUseWeights(kMaxWeight / 2 + 1, 2));
  EXPECT_EQ(kMaxWeight, MultiplyUseWeights(2, kMaxWeight / 2 + 1));
  EXPECT_EQ(kMaxWeight, MultiplyUseWeights(kMaxWeight, kMaxWeight));
}

TEST(UseWeights, IncludeCommentsAccumulateWeightedAndOrdinaryUses) {
  OneIncludeOrForwardDeclareLine line(std::nullopt, "\"header.h\"", -1, -1);
  line.AddSymbolUse("more_frequent", 40);
  line.AddSymbolUse("more_frequent", 2);
  line.AddSymbolUse("ordinary");
  EXPECT_EQ(42u, line.symbol_counts().at("more_frequent"));
  EXPECT_EQ(1u, line.symbol_counts().at("ordinary"));
}

TEST(UseWeights, IncludeCommentCountsSaturate) {
  OneIncludeOrForwardDeclareLine line(std::nullopt, "\"header.h\"", -1, -1);
  line.AddSymbolUse("symbol", kMaxWeight - 1);
  line.AddSymbolUse("symbol", 2);
  EXPECT_EQ(kMaxWeight, line.symbol_counts().at("symbol"));
}

TEST(UseWeights, CommentOrderUsesWeightsBeforeAlphabeticalOrder) {
  OneIncludeOrForwardDeclareLine line(std::nullopt, "\"header.h\"", -1, -1);
  line.AddSymbolUse("Alpha");
  line.AddSymbolUse("Zeta", 42);
  line.AddSymbolUse("Beta", 2);
  EXPECT_EQ((vector<string>{"Zeta", "Beta", "Alpha"}),
            internal::GetSymbolsSortedByFrequency(line.symbol_counts()));
}

TEST(UseWeights, QualifiedCommentNamesRetainWeightedOrderAndTies) {
  OneIncludeOrForwardDeclareLine line(std::nullopt, "\"header.h\"", -1, -1);
  line.AddSymbolUse("sample::Alpha (ptr only)", 2);
  line.AddSymbolUse("sample::Zeta", 42);
  line.AddSymbolUse("sample::Beta", 2);
  EXPECT_EQ((vector<string>{"sample::Zeta", "sample::Alpha (ptr only)",
                            "sample::Beta"}),
            internal::GetSymbolsSortedByFrequency(line.symbol_counts()));
}

}  // namespace
}  // namespace include_what_you_use
