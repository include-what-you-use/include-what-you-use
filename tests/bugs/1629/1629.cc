// IWYU_ARGS: -std=c++14
// IWYU_XFAIL

#include "foo.h"

int main() {
  Vector<int> v;
}

/**** IWYU_SUMMARY

(tests/bugs/1629/1629.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
