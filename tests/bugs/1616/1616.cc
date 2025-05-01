// IWYU_ARGS: -std=c++20
// IWYU_XFAIL

#include <utility>
#include <span>

template <typename T>
std::pair<T, T> foo(std::span<int>);

/**** IWYU_SUMMARY

(tests/bugs/1616/1616.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
