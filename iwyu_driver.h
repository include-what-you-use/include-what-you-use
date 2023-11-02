//===--- iwyu_driver.h - iwyu driver implementation -----------*- C++ -*---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_IWYU_DRIVER_H_
#define INCLUDE_WHAT_YOU_USE_IWYU_DRIVER_H_

#include <functional>
#include <memory>

namespace clang {
class FrontendAction;

namespace driver {
class ToolChain;
}
}

namespace include_what_you_use {

using clang::FrontendAction;
using clang::driver::ToolChain;

typedef std::function<std::unique_ptr<FrontendAction>(const ToolChain&)>
    ActionFactory;

// Use Clang's Driver to parse the command-line arguments, set up the state for
// the compilation, and execute the right action. IWYU action type is injected
// via factory callback.
bool ExecuteAction(int argc, const char** argv, ActionFactory make_iwyu_action);

}  // namespace include_what_you_use

#endif  // INCLUDE_WHAT_YOU_USE_IWYU_DRIVER_H_
