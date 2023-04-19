//===--- iwyu_driver.cc - iwyu driver implementation ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Driver, used to set up a clang CompilerInstance based on argc and
// argv.

// Everything below is adapted from clang/examples/clang-interpreter/main.cpp.

#include <cctype>
#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <utility>

#include "iwyu_globals.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/TargetParser/Triple.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Basic/DiagnosticFrontend.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Tool.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"

namespace llvm {
class LLVMContext;
}  // namespace llvm

using clang::CompilerInstance;
using clang::CompilerInvocation;
using clang::DiagnosticIDs;
using clang::DiagnosticOptions;
using clang::DiagnosticsEngine;
using clang::TextDiagnosticPrinter;
using clang::driver::Command;
using clang::driver::Compilation;
using clang::driver::Driver;
using clang::driver::JobList;
using llvm::ErrorOr;
using llvm::IntrusiveRefCntPtr;
using llvm::SmallString;
using llvm::SmallVector;
using llvm::SmallVectorImpl;
using llvm::MemoryBuffer;
using llvm::StringRef;
using llvm::cast;
using llvm::errs;
using llvm::isa;
using llvm::opt::ArgStringList;
using llvm::raw_svector_ostream;
using llvm::sys::getDefaultTargetTriple;
using std::set;
using std::unique_ptr;

namespace include_what_you_use {

namespace {

std::string GetExecutablePath(const char *Argv0) {
  // This just needs to be some symbol in the binary; C++ doesn't
  // allow taking the address of ::main however.
  void *main_addr = (void*) (intptr_t) GetExecutablePath;
  return llvm::sys::fs::getMainExecutable(Argv0, main_addr);
}

const char *SaveStringInSet(std::set<std::string> &SavedStrings, StringRef S) {
  return SavedStrings.insert(S.str()).first->c_str();
}

void ExpandArgsFromBuf(const char *Arg,
                       SmallVectorImpl<const char*> &ArgVector,
                       set<std::string> &SavedStrings) {
  const char *FName = Arg + 1;
  ErrorOr<unique_ptr<MemoryBuffer>> MemBufOrErr = MemoryBuffer::getFile(FName);
  if (!MemBufOrErr) {
    ArgVector.push_back(SaveStringInSet(SavedStrings, Arg));
    return;
  }

  const char *Buf = MemBufOrErr.get()->getBufferStart();
  char InQuote = ' ';
  std::string CurArg;

  for (const char *P = Buf; ; ++P) {
    if (*P == '\0' || (isspace(*P) && InQuote == ' ')) {
      if (!CurArg.empty()) {

        if (CurArg[0] != '@') {
          ArgVector.push_back(SaveStringInSet(SavedStrings, CurArg));
        } else {
          ExpandArgsFromBuf(CurArg.c_str(), ArgVector, SavedStrings);
        }

        CurArg = "";
      }
      if (*P == '\0')
        break;
      else
        continue;
    }

    if (isspace(*P)) {
      if (InQuote != ' ')
        CurArg.push_back(*P);
      continue;
    }

    if (*P == '"' || *P == '\'') {
      if (InQuote == *P)
        InQuote = ' ';
      else if (InQuote == ' ')
        InQuote = *P;
      else
        CurArg.push_back(*P);
      continue;
    }

    if (*P == '\\') {
      ++P;
      if (*P != '\0')
        CurArg.push_back(*P);
      continue;
    }
    CurArg.push_back(*P);
  }
}

void ExpandArgv(int argc, const char **argv,
                SmallVectorImpl<const char*> &ArgVector,
                set<std::string> &SavedStrings) {
  for (int i = 0; i < argc; ++i) {
    const char *Arg = argv[i];
    if (Arg[0] != '@') {
      ArgVector.push_back(SaveStringInSet(SavedStrings, std::string(Arg)));
      continue;
    }

    ExpandArgsFromBuf(Arg, ArgVector, SavedStrings);
  }
}

}  // anonymous namespace

CompilerInstance* CreateCompilerInstance(int argc, const char **argv) {
  std::string path = GetExecutablePath(argv[0]);
  IntrusiveRefCntPtr<DiagnosticOptions> diagnostic_options =
    new DiagnosticOptions;
  auto* diagnostic_client =
    new TextDiagnosticPrinter(errs(), &*diagnostic_options);

  IntrusiveRefCntPtr<DiagnosticIDs> diagnostic_id(new DiagnosticIDs());
  DiagnosticsEngine diagnostics(diagnostic_id, &*diagnostic_options,
                                diagnostic_client);
  Driver driver(path, getDefaultTargetTriple(), diagnostics);
  driver.setTitle("include what you use");

  // Expand out any response files passed on the command line
  set<std::string> SavedStrings;
  SmallVector<const char*, 256> args;

  ExpandArgv(argc, argv, args, SavedStrings);

  // Drop -save-temps arguments to avoid multiple compilation jobs.
  llvm::erase_if(args, [](const char *v) {
    StringRef arg(v);
    return arg.startswith("-save-temps") || arg.startswith("--save-temps");
  });

  // FIXME: This is a hack to try to force the driver to do something we can
  // recognize. We need to extend the driver library to support this use model
  // (basically, exactly one input, and the operation mode is hard wired).
  args.push_back("-fsyntax-only");

  unique_ptr<Compilation> compilation(driver.BuildCompilation(args));
  if (!compilation)
    return nullptr;

  ParseToolChain(compilation->getDefaultToolChain());

  // FIXME: This is copied from ASTUnit.cpp; simplify and eliminate.

  // We expect to get back exactly one command job, if we didn't something
  // failed. Extract that job from the compilation.
  const JobList& jobs = compilation->getJobs();
  if (jobs.size() != 1 || !isa<Command>(*jobs.begin())) {
    SmallString<256> msg;
    raw_svector_ostream out(msg);
    jobs.Print(out, "; ", true);
    diagnostics.Report(clang::diag::err_fe_expected_compiler_job) << out.str();
    return nullptr;
  }

  const Command& command = cast<Command>(*jobs.begin());
  if (StringRef(command.getCreator().getName()) != "clang") {
    diagnostics.Report(clang::diag::err_fe_expected_clang_command);
    return nullptr;
  }

  // Initialize a compiler invocation object from the clang (-cc1) arguments.
  const ArgStringList &cc_arguments = command.getArguments();
  std::shared_ptr<CompilerInvocation> invocation(new CompilerInvocation);
  CompilerInvocation::CreateFromArgs(*invocation, cc_arguments, diagnostics);
  invocation->getFrontendOpts().DisableFree = false;

  // Show the invocation, with -v.
  if (invocation->getHeaderSearchOpts().Verbose) {
    errs() << "clang invocation:\n";
    jobs.Print(errs(), "\n", true);
    errs() << "\n";
  }

  // FIXME: This is copied from cc1_main.cpp; simplify and eliminate.

  // Create a compiler instance to handle the actual work.
  // The caller will be responsible for freeing this.
  CompilerInstance* compiler = new CompilerInstance;
  compiler->setInvocation(invocation);

  // Create the compilers actual diagnostics engine.
  compiler->createDiagnostics();
  if (!compiler->hasDiagnostics())
    return nullptr;

  return compiler;
}

}  // namespace include_what_you_use
