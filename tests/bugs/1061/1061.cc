// IWYU_XFAIL
#include <chrono>

auto calc(std::chrono::seconds d)
{
    return d + std::chrono::seconds(1);
}

/**** IWYU_SUMMARY

(tests/bugs/1061/1061.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
