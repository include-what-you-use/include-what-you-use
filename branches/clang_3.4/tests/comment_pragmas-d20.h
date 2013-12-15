//===--- comment_pragmas-d20.h - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_COMMENT_PRAGMAS_D20_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_COMMENT_PRAGMAS_D20_H_

namespace { class CommentPragmasD20a {}; }

namespace Foo { namespace { class CommentPragmasD20b {}; } }

namespace { class CommentPragmasD20c {}; }

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_COMMENT_PRAGMAS_D20_H_
