// IWYU_ARGS: -I tests/bugs/1087 -include b.h
// IWYU_XFAIL

#include "1087.h"
void foo() {
}

/**** IWYU_SUMMARY

(tests/bugs/1087/1087.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
