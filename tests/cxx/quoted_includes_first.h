//===--- quoted_includes_first.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <iostream>
#include <list>
#include <map>
#include <memory>
#include "subdir/indirect_subdir.h"

std::unique_ptr<IndirectSubDirClass> CreateIndirectSubDirClass();

/**** IWYU_SUMMARY

tests/cxx/quoted_includes_first.h should add these lines:
class IndirectSubDirClass;

tests/cxx/quoted_includes_first.h should remove these lines:
- #include "subdir/indirect_subdir.h"  // lines XX-XX
- #include <iostream>  // lines XX-XX
- #include <list>  // lines XX-XX
- #include <map>  // lines XX-XX

The full include-list for tests/cxx/quoted_includes_first.h:
#include <memory>  // for unique_ptr
class IndirectSubDirClass;

***** IWYU_SUMMARY */
