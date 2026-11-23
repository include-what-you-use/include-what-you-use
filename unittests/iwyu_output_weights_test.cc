//===--- iwyu_output_weights_test.cc - weighted output tests --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_output.h"

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "clang/Basic/FileEntry.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/FileSystemOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "gtest/gtest.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/VirtualFileSystem.h"

namespace include_what_you_use {

class IwyuOutputTestPeer {
 public:
  static std::vector<OneUse>& Uses(IwyuFileInfo& file) {
    return file.symbol_uses_;
  }

  static void SetCandidates(OneUse& use,
                            const std::vector<std::string>& headers) {
    use.public_headers_ = headers;
    use.canonical_headers_ = headers;
  }

  static void ForceZeroWeight(OneUse& use) {
    use.weight_ = 0;
  }

  static void AppendMacro(IwyuFileInfo& file, clang::FileEntryRef header) {
    // Invalid locations avoid installing a global SourceManager. This uses
    // the same append path and ineligible OneUse form as ReportMacroUse.
    file.AppendUse(OneUse("MACRO", header, {}));
  }
};

namespace internal {
std::set<std::string> CalculateMinimalIncludes(
    const std::string& use_quoted_include,
    const std::set<std::string>& direct_includes,
    const std::set<std::string>& associated_desired_includes,
    std::vector<OneUse>* uses);
}  // namespace internal

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

struct RecordingListener : UseReportListener {
  void UseReported(IwyuFileInfo* file, size_t index) override {
    reports.emplace_back(file, index);
  }
  std::vector<std::pair<IwyuFileInfo*, size_t>> reports;
};

class WeightedReporting : public ::testing::Test {
 protected:
  void SetUp() override {
    auto filesystem =
        llvm::makeIntrusiveRefCnt<llvm::vfs::InMemoryFileSystem>();
    ASSERT_TRUE(filesystem->addFile("/weighted/header.h", 0,
                                    llvm::MemoryBuffer::getMemBuffer("")));
    files_ = std::make_unique<clang::FileManager>(clang::FileSystemOptions(),
                                                  filesystem);
    header_ = files_->getOptionalFileRef("/weighted/header.h");
    ASSERT_TRUE(header_);
    file_ =
        std::make_unique<IwyuFileInfo>(std::nullopt, nullptr, "\"input.cc\"");
    SetUseReportListener(&listener_);
  }

  void TearDown() override {
    SetUseReportListener(nullptr);
  }

  void Report(const std::string& name) {
    file_->ReportFullSymbolUse({}, header_, name);
  }

  std::vector<OneUse>& Uses() {
    return IwyuOutputTestPeer::Uses(*file_);
  }

  OneUse MakeUse(const std::vector<std::string>& candidates, uint64_t weight) {
    OneUse use("Symbol", header_, {});
    IwyuOutputTestPeer::SetCandidates(use, candidates);
    use.set_weight(weight);
    return use;
  }

  std::set<std::string> Select(std::vector<OneUse>* uses) {
    return internal::CalculateMinimalIncludes("\"input.cc\"", {}, {}, uses);
  }

  void ExpectBoundary() {
    file_->ReportUseAgain(0, 3);
    ASSERT_EQ(4u, Uses().size());
    EXPECT_EQ(2u, Uses()[1].weight());
    EXPECT_EQ(3u, Uses()[3].weight());
    EXPECT_EQ(Uses()[0].symbol_name(), Uses()[3].symbol_name());
  }

  void ExpectEquivalentSelection(std::vector<OneUse> weighted,
                                 const std::set<std::string>& expected) {
    std::vector<OneUse> expanded;
    for (const OneUse& use : weighted) {
      for (uint64_t i = 0; i < use.weight(); ++i) {
        OneUse copy = use;
        copy.set_weight(1);
        expanded.push_back(std::move(copy));
      }
    }
    EXPECT_EQ(expected, Select(&weighted));
    EXPECT_EQ(expected, Select(&expanded));
  }

  std::unique_ptr<clang::FileManager> files_;
  clang::OptionalFileEntryRef header_;
  std::unique_ptr<IwyuFileInfo> file_;
  RecordingListener listener_;
};

TEST_F(WeightedReporting, ListenerSeesOriginalReportsButNotRepeats) {
  Report("Alpha");
  file_->ReportUseAgain(0, 2);
  Report("Beta");
  ASSERT_EQ(2u, listener_.reports.size());
  EXPECT_EQ(file_.get(), listener_.reports[0].first);
  EXPECT_EQ(0u, listener_.reports[0].second);
  EXPECT_EQ(2u, listener_.reports[1].second);
  SetUseReportListener(nullptr);
  Report("Gamma");
  EXPECT_EQ(2u, listener_.reports.size());
}

TEST_F(WeightedReporting, CoalescesRepeatsAndSaturatesTheirWeight) {
  Report("Alpha");
  file_->ReportUseAgain(0, 2);
  file_->ReportUseAgain(0, 3);
  ASSERT_EQ(2u, Uses().size());
  EXPECT_EQ(1u, Uses()[0].weight());
  EXPECT_EQ(5u, Uses()[1].weight());
  file_->ReportUseAgain(0, kMaxWeight);
  EXPECT_EQ(kMaxWeight, Uses()[1].weight());
}

TEST_F(WeightedReporting, NewSymbolEndsCoalescingWindow) {
  Report("Alpha");
  file_->ReportUseAgain(0, 2);
  Report("Beta");
  ExpectBoundary();
}

TEST_F(WeightedReporting, MacroEndsCoalescingWindowWithoutNotifyingListener) {
  Report("Alpha");
  file_->ReportUseAgain(0, 2);
  IwyuOutputTestPeer::AppendMacro(*file_, *header_);
  ExpectBoundary();
  EXPECT_EQ(1u, listener_.reports.size());
}

TEST_F(WeightedReporting, IncludeEndsCoalescingWindowWithoutNotifyingListener) {
  Report("Alpha");
  file_->ReportUseAgain(0, 2);
  file_->ReportIncludeFileUse(header_, "\"header.h\"", {});
  ExpectBoundary();
  EXPECT_EQ(1u, listener_.reports.size());
}

TEST_F(WeightedReporting, RejectsZeroWeightAndInvalidIndex) {
  Report("Alpha");
  EXPECT_DEATH_IF_SUPPORTED(file_->ReportUseAgain(0, 0), "weight.*positive");
  EXPECT_DEATH_IF_SUPPORTED(file_->ReportUseAgain(Uses().size(), 1), "index");
  EXPECT_DEATH_IF_SUPPORTED(Uses()[0].set_weight(0), "weight.*positive");
}

TEST_F(WeightedReporting, RejectsMacroIncludeAndRepeatedEntries) {
  Report("Alpha");
  IwyuOutputTestPeer::AppendMacro(*file_, *header_);
  file_->ReportIncludeFileUse(header_, "\"header.h\"", {});
  file_->ReportUseAgain(0, 2);
  for (size_t index : {1u, 2u, 3u}) {
    EXPECT_DEATH_IF_SUPPORTED(file_->ReportUseAgain(index, 1),
                              "original listener-reported");
  }
}

TEST_F(WeightedReporting, RejectsRepeatingAfterHeaderSelection) {
  Report("Alpha");
  Uses()[0].set_suggested_header("\"header.h\"");
  EXPECT_DEATH_IF_SUPPORTED(file_->ReportUseAgain(0, 1), "header selection");
}

TEST_F(WeightedReporting, CanonicalPreferenceCountsWeightedAndOrdinaryUses) {
  ExpectEquivalentSelection(
      {MakeUse({"\"a.h\"", "\"b.h\""}, 1), MakeUse({"\"b.h\"", "\"a.h\""}, 5)},
      {"\"b.h\""});
}

TEST_F(WeightedReporting, TotalWeightTakesPrecedenceOverCanonicalPreference) {
  ExpectEquivalentSelection(
      {MakeUse({"\"a.h\"", "\"b.h\""}, 5), MakeUse({"\"b.h\"", "\"c.h\""}, 4),
       MakeUse({"\"c.h\"", "\"a.h\""}, 10)},
      {"\"a.h\"", "\"b.h\""});
}

TEST_F(WeightedReporting, RepeatedReportsPreserveHeaderChoice) {
  Report("Alpha");
  IwyuOutputTestPeer::SetCandidates(Uses()[0], {"\"a.h\"", "\"b.h\""});
  file_->ReportUseAgain(0, 2);
  Report("Beta");
  IwyuOutputTestPeer::SetCandidates(Uses()[2], {"\"b.h\"", "\"a.h\""});
  file_->ReportUseAgain(2, 10);
  ASSERT_EQ(2u, listener_.reports.size());
  EXPECT_EQ((std::set<std::string>{"\"b.h\""}), Select(&Uses()));
  for (const OneUse& use : Uses())
    EXPECT_EQ("\"b.h\"", use.suggested_header());
}

TEST_F(WeightedReporting, SaturationLeavesHeaderTiesToMapOrder) {
  std::vector<OneUse> uses = {MakeUse({"\"a.h\"", "\"b.h\""}, kMaxWeight),
                              MakeUse({"\"b.h\"", "\"a.h\""}, kMaxWeight),
                              MakeUse({"\"b.h\"", "\"a.h\""}, 1)};
  EXPECT_EQ((std::set<std::string>{"\"a.h\""}), Select(&uses));
}

TEST_F(WeightedReporting, RejectsEmptyHeaderCountsInsteadOfDereferencingEnd) {
  std::vector<OneUse> uses = {MakeUse({"\"a.h\"", "\"b.h\""}, 1)};
  // Simulate a broken producer bypassing the positive-weight precondition.
  IwyuOutputTestPeer::ForceZeroWeight(uses[0]);
  EXPECT_DEATH_IF_SUPPORTED(Select(&uses), "No canonical header candidates");
}

}  // namespace
}  // namespace include_what_you_use
