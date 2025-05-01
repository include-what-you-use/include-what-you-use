// IWYU_XFAIL
#include <iostream>

#include "vector.h"

int main() {
  Vector<int> v{1, 2, 3};

  for (int x : v) {
    std::cout << x << std::endl;
  }
}

/**** IWYU_SUMMARY

(tests/bugs/1349/1349.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
