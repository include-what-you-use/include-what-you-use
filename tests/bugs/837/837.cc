// IWYU_XFAIL

#include "fooA.h"
#include "fooB.h"

int main() {
  auto pFooB{getPointerFooB()};

  // std::unique_ptr<fooB> pFooB{ getPointerFooB()};
  return 0;
}

/**** IWYU_SUMMARY

(tests/bugs/837/837.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
