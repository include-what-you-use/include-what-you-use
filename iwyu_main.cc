//===--- iwyu_main.cc - iwyu main function --------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <cstdlib>
#include <memory>

#include "iwyu.h"
#include "iwyu_driver.h"
#include "iwyu_globals.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"

// The lambda passed to ExecuteAction mentions FrontendAction, but it shouldn't
// be needed here.
// IWYU pragma: no_include "clang/Frontend/FrontendAction.h"

namespace clang {
namespace driver {
class ToolChain;
}  // namespace driver
}  // namespace clang

int main(int argc, char** argv) {
  using clang::driver::ToolChain;
  using include_what_you_use::ExecuteAction;
  using include_what_you_use::IwyuAction;
  using include_what_you_use::OptionsParser;

  llvm::llvm_shutdown_obj scoped_shutdown;

  // X86 target is required to parse Microsoft inline assembly, so we hope it's
  // part of all targets. Clang parser will complain otherwise.
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();

  // The command line should look like
  //   path/to/iwyu -Xiwyu --verbose=4 [-Xiwyu --other_iwyu_flag]... \
  //       CLANG_FLAGS... foo.cc
  OptionsParser options_parser(argc, argv);
  if (!ExecuteAction(options_parser.clang_argc(), options_parser.clang_argv(),
                     [](const ToolChain& toolchain) {
                       return std::make_unique<IwyuAction>(toolchain);
                     })) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
