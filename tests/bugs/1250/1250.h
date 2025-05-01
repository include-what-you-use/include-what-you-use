#pragma once

#include "bar.h"

template <typename T>
T get_begin(T const (&x)[10]) {
  // IWYU: begin is...*<iterator>
  return *std::begin(x);
}

/**** IWYU_SUMMARY

tests/bugs/1250/1250.h should add these lines:
#include <iterator>

tests/bugs/1250/1250.h should remove these lines:
- #include "bar.h"  // lines XX-XX

The full include-list for tests/bugs/1250/1250.h:
#include <iterator>  // for begin

***** IWYU_SUMMARY */
