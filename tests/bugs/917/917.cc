// IWYU_ARGS: -std=c++20
// IWYU_XFAIL

#include <concepts>

#include "mytype.h"

static_assert(!std::constructible_from<MyType, int>);

int main() {}

/**** IWYU_SUMMARY

(tests/bugs/917/917.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
