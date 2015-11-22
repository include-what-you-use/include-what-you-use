#include "iwyu_globals.h"
#include "gtest/gtest.h"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  include_what_you_use::InitGlobalsAndFlagsForTesting();
  return RUN_ALL_TESTS();
}
