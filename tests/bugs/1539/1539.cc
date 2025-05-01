// IWYU_ARGS: -stdlib=libstdc++
// IWYU_XFAIL
#include "my_vector.h"

int main() {
  MyVector<int> v;
  bool at_end = v.begin() == v.end();
  return 0;
}

/**** IWYU_SUMMARY

(tests/bugs/1539/1539.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
