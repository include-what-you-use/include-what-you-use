// macro_and_template.cc
#include "macro_and_template-d1.h"
#include "macro_and_template-d2.h"


void hello() {
  MACRO().func<Test>();
}

/**** IWYU_SUMMARY

(tests/cxx/macro_and_template.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
