// IWYU_XFAIL

enum Foo {
#include "enumerators.def"
};

/**** IWYU_SUMMARY

(tests/bugs/1743/1743.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
