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
#include "iwyu_driver.h"

#include <cctype>
#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticFrontend.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Driver/Action.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Job.h"
#include "clang/Driver/Tool.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendOptions.h"
#include "clang/FrontendTool/Utils.h"
#include "clang/Lex/HeaderSearchOptions.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "iwyu_verrs.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Option/Option.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/TargetParser/Host.h"

// TODO: Clean out pragmas as IWYU improves.
// IWYU pragma: no_include "clang/Basic/LLVM.h"
// IWYU pragma: no_include "llvm/ADT/iterator.h"

namespace include_what_you_use {

using clang::CompilerInstance;
using clang::CompilerInvocation;
using clang::DiagnosticOptions;
using clang::DiagnosticsEngine;
using clang::FrontendAction;
using clang::PreprocessorOptions;
using clang::driver::Action;
using clang::driver::Command;
using clang::driver::Compilation;
using clang::driver::Driver;
using clang::driver::JobList;
using llvm::ArrayRef;
using llvm::ErrorOr;
using llvm::IntrusiveRefCntPtr;
using llvm::MemoryBuffer;
using llvm::SmallVector;
using llvm::SmallVectorImpl;
using llvm::StringRef;
using llvm::errs;
using llvm::opt::ArgStringList;
using llvm::raw_ostream;
using llvm::raw_string_ostream;
using llvm::sys::getDefaultTargetTriple;
using std::set;
using std::unique_ptr;

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

bool HasPreprocessOnlyArgs(ArrayRef<const char*> args) {
  auto is_preprocess_only = [](StringRef arg) {
    // Handle GCC spelling.
    if (arg == "-E")
      return true;

    // Handle MSVC spellings.
    if (arg == "/E" || arg == "/EP" || arg == "/P")
      return true;

    return false;
  };

  return llvm::any_of(args, is_preprocess_only);
}

// Print the command prefixed by its action class
raw_ostream& operator<<(raw_ostream& s, const Command& job) {
  s << "(" << job.getSource().getClassName() << ")";
  job.Print(s, "", false);
  return s;
}

std::string JobsToString(const JobList& jobs, const char* sep) {
  std::string msg;
  raw_string_ostream out(msg);
  for (const Command& job : jobs) {
    out << job << sep;
  }
  return msg;
}

std::string JobsToString(ArrayRef<const Command*> jobs, const char* sep) {
  std::string msg;
  raw_string_ostream out(msg);
  for (const Command* job : jobs) {
    out << *job << sep;
  }
  return msg;
}

std::vector<const Command*> FilterJobs(const JobList& jobs) {
  std::vector<const Command*> res;
  for (const Command& job : jobs) {
    const Action& action = job.getSource();
    if (action.getKind() != Action::CompileJobClass &&
        action.getKind() != Action::PreprocessJobClass) {
      VERRS(2) << "warning: ignoring unsupported job type: "
               << action.getClassName() << "\n";
      continue;
    }

    Action::OffloadKind offload_kind = action.getOffloadingDeviceKind();
    if (offload_kind != Action::OFK_None) {
      VERRS(2) << "warning: ignoring offload job for device toolchain: "
               << action.GetOffloadKindName(offload_kind) << "\n";
      continue;
    }

    StringRef tool = job.getCreator().getName();
    if (tool != "clang") {
      VERRS(2) << "warning: ignoring job from unexpected tool: " << tool
               << "\n";
      continue;
    }

    res.push_back(&job);
  }
  return res;
}

}  // anonymous namespace

bool ExecuteAction(int argc, const char** argv,
                   ActionFactory make_iwyu_action) {
  std::string path = GetExecutablePath(argv[0]);

  IntrusiveRefCntPtr<DiagnosticsEngine> diagnostics =
      CompilerInstance::createDiagnostics(new DiagnosticOptions,
                                          /*Client=*/nullptr,
                                          /*ShouldOwnClient=*/true,
                                          /*CodeGenOpts=*/nullptr);

  Driver driver(path, getDefaultTargetTriple(), *diagnostics);
  driver.setTitle("include what you use");

  // Expand out any response files passed on the command line
  set<std::string> SavedStrings;
  SmallVector<const char*, 256> args;

  ExpandArgv(argc, argv, args, SavedStrings);

  // Drop -save-temps arguments to avoid multiple compilation jobs.
  llvm::erase_if(args, [](StringRef arg) {
    return arg.starts_with("-save-temps") || arg.starts_with("--save-temps");
  });

  // FIXME: This is a hack to try to force the driver to do something we can
  // recognize. We need to extend the driver library to support this use model
  // (basically, exactly one input, and the operation mode is hard wired).

  // Add -fsyntax-only to avoid code generation, unless user asked for
  // preprocessing-only.
  if (!HasPreprocessOnlyArgs(args)) {
    args.push_back("-fsyntax-only");
    args.push_back("-Qunused-arguments");
  }

  // Build a compilation, get the job list and filter out irrelevant jobs.
  unique_ptr<Compilation> compilation(driver.BuildCompilation(args));
  if (!compilation)
    return false;

  const JobList& jobs = compilation->getJobs();
  std::vector<const Command*> filtered_jobs = FilterJobs(jobs);
  if (filtered_jobs.empty()) {
    diagnostics->Report(clang::diag::err_fe_expected_compiler_job)
        << JobsToString(jobs, "; ");
    return false;
  }

  // If we have more than one job after filtering, there's a good chance
  // FilterJobs could be improved to prune the extra jobs. Log them at level 2.
  if (filtered_jobs.size() > 1 && ShouldPrint(2)) {
    auto extra_jobs = ArrayRef<const Command*>(filtered_jobs).drop_front(1);
    errs() << "warning: ignoring " << extra_jobs.size() << " extra jobs:\n"
           << JobsToString(extra_jobs, "\n") << "\n";
  }

  // Initialize a compiler invocation object from the clang (-cc1) arguments.
  const Command& command = *filtered_jobs[0];
  const ArgStringList& cc_arguments = command.getArguments();
  std::shared_ptr<CompilerInvocation> invocation(new CompilerInvocation);
  CompilerInvocation::CreateFromArgs(*invocation, cc_arguments, *diagnostics);
  invocation->getFrontendOpts().DisableFree = false;

  // Show the invocation, with -v.
  if (invocation->getHeaderSearchOpts().Verbose) {
    errs() << "clang invocation:\n" << JobsToString(jobs, "\n") << "\n";
  }

  // Reject attempts at using precompiled headers.
  const PreprocessorOptions& opts = invocation->getPreprocessorOpts();
  if (!opts.ImplicitPCHInclude.empty() || !opts.PCHThroughHeader.empty()) {
    errs() << "error: include-what-you-use does not support PCH\n";
    return false;
  }

  // FIXME: This is copied from cc1_main.cpp; simplify and eliminate.

  // Create a compiler instance to handle the actual work.
  unique_ptr<CompilerInstance> compiler(new CompilerInstance);
  compiler->setInvocation(invocation);
  // It's tempting to reuse the DiagnosticsEngine we created above, but we need
  // to create a new one to get the options produced by the compiler invocation.
  compiler->createDiagnostics();

  unique_ptr<FrontendAction> action;
  switch (command.getSource().getKind()) {
    case Action::PreprocessJobClass:
      // Let Clang create a preprocessing frontend action.
      action = CreateFrontendAction(*compiler);
      break;

    case Action::CompileJobClass:
      // Drop compiler job and run IWYU instead.
      action = make_iwyu_action(compilation->getDefaultToolChain());
      break;

    default:
      errs() << "error: expected compiler or preprocessor job, found: "
             << command << "\n";
      return false;
  }

  // Run the action.
  return compiler->ExecuteAction(*action);
}

}  // namespace include_what_you_use
