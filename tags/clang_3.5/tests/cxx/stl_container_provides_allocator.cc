//===--- stl_container_provides_allocator.cc - test input file for iwyu ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that STL containers provide from <memory> only std::allocator as
// default template argument and nothing else.  For example, when you use
// std::vector<int>, it has default allocator and you don't need to
// include <memory>, because it is already included [transitively] by <vector>.
// But if you use something from <memory> explicitly, you need to include it
// yourself, <vector> doesn't reexport everything from <memory>.
//
// Related test: precomputed_tpl_args.cc.

#include <memory>
#include <vector>
// Unnecessary include, so that symbols for each include are printed in full
// include-list.
#include <stdio.h>

void foo() {
  int buffer[4];
  std::uninitialized_fill(buffer, buffer + 4, 0);

  std::vector<int> v;
}

/**** IWYU_SUMMARY

tests/cxx/stl_container_provides_allocator.cc should add these lines:

tests/cxx/stl_container_provides_allocator.cc should remove these lines:
- #include <stdio.h>  // lines XX-XX

The full include-list for tests/cxx/stl_container_provides_allocator.cc:
#include <memory>  // for uninitialized_fill
#include <vector>  // for vector

***** IWYU_SUMMARY */
