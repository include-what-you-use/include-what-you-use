// Copyright 2011 Google Inc. All Rights Reserved.
// Author: csilvers@google.com (Craig Silverstein)

// Driver, used to set up a clang CompilerInstance based on argc and
// argv.

// Everything below is adapted from clang/examples/clang-interpreter/main.cpp.

#include <ctype.h>
#include <stdint.h>
#include <set>
#include <string>
#include <utility>

#include "llvm/ADT/ArrayRef.h"  // IWYU pragma: keep
#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/PathV1.h"
#include "llvm/Support/system_error.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Tool.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/DiagnosticOptions.h"
#include "clang/Frontend/FrontendDiagnostic.h"  // IWYU pragma: keep
#include "clang/Frontend/TextDiagnosticPrinter.h"

namespace llvm {
class LLVMContext;
}  // namespace llvm

using clang::ASTFrontendAction;
using clang::CompilerInstance;
using clang::CompilerInvocation;
using clang::Diagnostic;
using clang::DiagnosticIDs;
using clang::DiagnosticOptions;
using clang::TextDiagnosticPrinter;
using clang::driver::ArgStringList;
using clang::driver::Command;
using clang::driver::Compilation;
using clang::driver::Driver;
using clang::driver::JobList;
using llvm::IntrusiveRefCntPtr;
using llvm::LLVMContext;
using llvm::OwningPtr;
using llvm::SmallString;
using llvm::SmallVector;
using llvm::SmallVectorImpl;
using llvm::MemoryBuffer;
using llvm::StringRef;
using llvm::errs;
using llvm::raw_svector_ostream;
using llvm::sys::getHostTriple;
using llvm::sys::Path;
using std::set;

namespace include_what_you_use {

namespace {

Path GetExecutablePath(const char *Argv0) {
  // This just needs to be some symbol in the binary; C++ doesn't
  // allow taking the address of ::main however.
  void *main_addr = (void*) (intptr_t) GetExecutablePath;
  return Path::GetMainExecutable(Argv0, main_addr);
}

const char *SaveStringInSet(std::set<std::string> &SavedStrings, StringRef S) {
  return SavedStrings.insert(S).first->c_str();
}

void ExpandArgsFromBuf(const char *Arg,
                       SmallVectorImpl<const char*> &ArgVector,
                       set<std::string> &SavedStrings) {
  const char *FName = Arg + 1;
  OwningPtr<MemoryBuffer> MemBuf;
  if (MemoryBuffer::getFile(FName, MemBuf)) {
    ArgVector.push_back(SaveStringInSet(SavedStrings, Arg));
    return;
  }

  const char *Buf = MemBuf->getBufferStart();
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

}  // unnamed namespace

CompilerInstance* CreateCompilerInstance(int argc, const char **argv) {
  void* main_addr = (void*) (intptr_t) GetExecutablePath;
  Path path = GetExecutablePath(argv[0]);
  TextDiagnosticPrinter* diagnostic_client =
    new TextDiagnosticPrinter(errs(), DiagnosticOptions());

  IntrusiveRefCntPtr<DiagnosticIDs> diagnostic_id(new DiagnosticIDs());
  Diagnostic diagnostics(diagnostic_id, diagnostic_client);
  Driver driver(path.str(), getHostTriple(), "a.out",
                false, false, diagnostics);
  driver.setTitle("include what you use");

  // Expand out any response files passed on the command line
  set<std::string> SavedStrings;
  SmallVector<const char*, 256> args;

  ExpandArgv(argc, argv, args, SavedStrings);

  // FIXME: This is a hack to try to force the driver to do something we can
  // recognize. We need to extend the driver library to support this use model
  // (basically, exactly one input, and the operation mode is hard wired).
  args.push_back("-fsyntax-only");
  OwningPtr<Compilation> compilation(driver.BuildCompilation(args));
  if (!compilation)
    return NULL;

  // FIXME: This is copied from ASTUnit.cpp; simplify and eliminate.

  // We expect to get back exactly one command job, if we didn't something
  // failed. Extract that job from the compilation.
  const JobList& jobs = compilation->getJobs();
  if (jobs.size() != 1 || !isa<Command>(*jobs.begin())) {
    SmallString<256> msg;
    raw_svector_ostream out(msg);
    compilation->PrintJob(out, compilation->getJobs(), "; ", true);
    diagnostics.Report(clang::diag::err_fe_expected_compiler_job) << out.str();
    return NULL;
  }

  const Command *command = cast<Command>(*jobs.begin());
  if (StringRef(command->getCreator().getName()) != "clang") {
    diagnostics.Report(clang::diag::err_fe_expected_clang_command);
    return NULL;
  }

  // Initialize a compiler invocation object from the clang (-cc1) arguments.
  const ArgStringList &cc_arguments = command->getArguments();
  const char** args_start = const_cast<const char**>(cc_arguments.data());
  const char** args_end = args_start + cc_arguments.size();
  OwningPtr<CompilerInvocation> invocation(new CompilerInvocation);
  CompilerInvocation::CreateFromArgs(*invocation,
                                     args_start, args_end, diagnostics);
  invocation->getFrontendOpts().DisableFree = false;

  // Show the invocation, with -v.
  if (invocation->getHeaderSearchOpts().Verbose) {
    errs() << "clang invocation:\n";
    compilation->PrintJob(errs(), compilation->getJobs(), "\n", true);
    errs() << "\n";
  }

  // FIXME: This is copied from cc1_main.cpp; simplify and eliminate.

  // Create a compiler instance to handle the actual work.
  // The caller will be responsible for freeing this.
  CompilerInstance* compiler = new CompilerInstance;
  compiler->setInvocation(invocation.take());

  // Create the compilers actual diagnostics engine.
  compiler->createDiagnostics(args_end - args_start, args_start);
  if (!compiler->hasDiagnostics())
    return NULL;

  // Infer the builtin include path if unspecified.
  if (compiler->getHeaderSearchOpts().UseBuiltinIncludes &&
      compiler->getHeaderSearchOpts().ResourceDir.empty())
    compiler->getHeaderSearchOpts().ResourceDir =
      CompilerInvocation::GetResourcesPath(argv[0], main_addr);

  return compiler;
}

}  // namespace include_what_you_use
