//===--- 1617.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -stdlib=libc++
// IWYU_XFAIL
#include "item.h"
#include <tuple>

void process(std::tuple<Item*> foo) {
    auto [item] = foo;
    item->process();
}

/**** IWYU_SUMMARY

(tests/bugs/1617/1617.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
