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
#include "subfolder/indirect_subfolder.h"

std::unique_ptr<IndirectSubfolderClass> CreateIndirectSubfolderClass();

/**** IWYU_SUMMARY

tests/cxx/quoted_includes_first.h should add these lines:
class IndirectSubfolderClass;

tests/cxx/quoted_includes_first.h should remove these lines:
- #include "subfolder/indirect_subfolder.h"  // lines XX-XX
- #include <iostream>  // lines XX-XX
- #include <list>  // lines XX-XX
- #include <map>  // lines XX-XX

The full include-list for tests/cxx/quoted_includes_first.h:
#include <memory>  // for unique_ptr
class IndirectSubfolderClass;

***** IWYU_SUMMARY */
