//===--- iwyu_verrs_test.cc - test iwyu_verrs.h ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests for the iwyu_verrs module.
// Only tests pure functions that do not require Clang infrastructure.

#include "iwyu_verrs.h"

#include "gtest/gtest.h"

namespace include_what_you_use {

namespace {

// Fixture to save and restore verbose level between tests.
class IwyuVerrsTest : public ::testing::Test {
 protected:
  void SetUp() override {
    saved_level_ = GetVerboseLevel();
  }

  void TearDown() override {
    SetVerboseLevel(saved_level_);
  }

 private:
  int saved_level_;
};

TEST_F(IwyuVerrsTest, SetGetVerboseLevelRoundtrip) {
  SetVerboseLevel(5);
  EXPECT_EQ(5, GetVerboseLevel());

  SetVerboseLevel(0);
  EXPECT_EQ(0, GetVerboseLevel());

  SetVerboseLevel(10);
  EXPECT_EQ(10, GetVerboseLevel());
}

TEST_F(IwyuVerrsTest, ShouldPrintAtExactLevel) {
  SetVerboseLevel(3);
  EXPECT_TRUE(ShouldPrint(3));
}

TEST_F(IwyuVerrsTest, ShouldPrintBelowLevel) {
  SetVerboseLevel(3);
  EXPECT_TRUE(ShouldPrint(1));
  EXPECT_TRUE(ShouldPrint(2));
}

TEST_F(IwyuVerrsTest, ShouldPrintAboveLevelReturnsFalse) {
  SetVerboseLevel(3);
  EXPECT_FALSE(ShouldPrint(4));
  EXPECT_FALSE(ShouldPrint(5));
}

TEST_F(IwyuVerrsTest, ShouldPrintAtZeroLevel) {
  SetVerboseLevel(0);
  EXPECT_TRUE(ShouldPrint(0));
  EXPECT_FALSE(ShouldPrint(1));
}

}  // namespace
}  // namespace include_what_you_use
