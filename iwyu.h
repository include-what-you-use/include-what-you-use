//===--- iwyu.h - iwyu main module - C++ ----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "clang/Frontend/FrontendAction.h"

namespace clang {
namespace driver {
class ToolChain;
}
}  // namespace clang

namespace include_what_you_use {

using clang::ASTConsumer;
using clang::ASTFrontendAction;
using clang::CompilerInstance;
using clang::driver::ToolChain;
using llvm::StringRef;

// We use an ASTFrontendAction to hook up IWYU with Clang.
class IwyuAction : public ASTFrontendAction {
 public:
  explicit IwyuAction(const ToolChain& toolchain);

 protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance& compiler,
                                                 StringRef) override;

 private:
  // ToolChain is not copyable, but it's owned by Compilation which has the same
  // lifetime as CompilerInstance, so it should be alive for as long as we are.
  const ToolChain& toolchain_;
};

}  // namespace include_what_you_use
