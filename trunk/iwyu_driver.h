//===--- iwyu_driver.h - iwyu driver implementation -----------*- C++ -*---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_DRIVER_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_DRIVER_H_

namespace clang {
class CompilerInstance;
}

namespace include_what_you_use {

// Creates a CompilerInstance object based on the commandline
// arguments, or NULL if there's an error of some sort.
clang::CompilerInstance* CreateCompilerInstance(int argc, const char **argv);

}  // namespace include_what_you_use

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_DRIVER_H_
