// IWYU_XFAIL

// Swapping the order of these two includes makes the testcase pass, so IWYU
// behavior is a factor of include order here (because of dependencies between
// the headers.)
#include "include2.h"
#include "include1.h"

int main(int argc, char* argv[]) {
  importantthing s1;
  something2 s3;
  s1.x = OPTION1;
  s1.y = OPTION2;
  return 0;
}

/**** IWYU_SUMMARY

(tests/bugs/411/411.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
