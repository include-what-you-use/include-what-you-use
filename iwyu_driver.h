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

#include <memory>
#include "llvm/Option/ArgList.h"

namespace llvm {
  namespace opt {
    class InputArgList;
  }
}
namespace clang {
  class CompilerInstance;
  namespace driver {
    class Compilation;
    class Driver;
  }
}

namespace include_what_you_use {

class IwyuDriver {
public:
  IwyuDriver(int argc, const char **argv);
  ~IwyuDriver();
  std::shared_ptr<clang::CompilerInstance> getCompilerInstance();
  std::shared_ptr<clang::driver::Compilation> getCompilation();
  const llvm::opt::ArgList & getArgs();
  private:
  // Creates a CompilerInstance object based on the commandline
  // arguments, or NULL if there's an error of some sort.
  void CreateCompilerInstance(int argc, const char **argv);
  std::shared_ptr<clang::CompilerInstance> compilerInstance;
  std::shared_ptr<clang::driver::Compilation> compilation;
  std::shared_ptr<clang::driver::Driver> driver;
  std::shared_ptr<llvm::opt::InputArgList> Args;
};

}  // namespace include_what_you_use

#endif  // INCLUDE_WHAT_YOU_USE_IWYU_DRIVER_H_
