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
#include "clang/Basic/DiagnosticFrontend.h"  // IWYU pragma: keep
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
#include "clang/Options/OptionUtils.h"
#include "iwyu_port.h"
#include "iwyu_verrs.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Option/Option.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/VirtualFileSystem.h"
#include "llvm/TargetParser/Host.h"

// TODO: Clean out pragmas as IWYU improves.
// IWYU pragma: no_include "clang/Basic/DiagnosticFrontendInterface.inc"
// IWYU pragma: no_include "clang/Basic/LLVM.h"
// IWYU pragma: no_include "llvm/ADT/iterator.h"

namespace include_what_you_use {

using clang::CompilerInstance;
using clang::CompilerInvocation;
using clang::DiagnosticOptions;
using clang::DiagnosticsEngine;
using clang::FrontendAction;
using clang::GetResourcesPath;
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
using llvm::SmallString;
using llvm::SmallVector;
using llvm::SmallVectorImpl;
using llvm::StringRef;
using llvm::errs;
using llvm::opt::ArgStringList;
using llvm::raw_ostream;
using llvm::raw_string_ostream;
using llvm::sys::getDefaultTargetTriple;
using llvm::vfs::FileSystem;
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

bool HasArg(ArrayRef<const char*> args, const char* needle) {
  return llvm::any_of(args, [&](StringRef arg) {
    return arg == needle;
  });
}

bool HasArgPrefix(ArrayRef<const char*> args, const char* prefix) {
  return llvm::any_of(args, [&](StringRef arg) {
    return arg.starts_with(prefix);
  });
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
        action.getKind() != Action::PrecompileJobClass &&
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

static std::string ComputeCustomResourceDir(StringRef iwyu_executable_path) {
  // There are basically four ways this can go...
  StringRef iwyu_resource_binary_path(IWYU_RESOURCE_BINARY_PATH);
  StringRef iwyu_resource_dir(IWYU_RESOURCE_DIR);
  if (iwyu_resource_binary_path.empty() && iwyu_resource_dir.empty()) {
    // 1. Neither are specified, so don't add explicit resource dir, and let
    // Clang driver do its default based on IWYU executable path (with
    // CLANG_RESOURCE_DIR in clang-20 and later).
    return std::string();
  }

  if (iwyu_resource_dir.empty()) {
    CHECK_(!iwyu_resource_binary_path.empty());
    // 2. Only IWYU_RESOURCE_BINARY_PATH specified, pass it to Driver to run it
    // through the default algorithm (with CLANG_RESOURCE_DIR in clang-20 and
    // later).
    return GetResourcesPath(iwyu_resource_binary_path);
  }

  CHECK_(!iwyu_resource_dir.empty());
  if (iwyu_resource_binary_path.empty()) {
    // 3. Only IWYU_RESOURCE_DIR specified. Default binary path to IWYU
    // executable path and carry on to (4).
    iwyu_resource_binary_path = iwyu_executable_path;
  }

  // 4. Both IWYU_RESOURCE_BINARY_PATH and IWYU_RESOURCE_DIR specified, join
  // them to form a custom resource dir.
  StringRef dir = llvm::sys::path::parent_path(iwyu_resource_binary_path);
  SmallString<128> res(dir);
  llvm::sys::path::append(res, iwyu_resource_dir);
  llvm::sys::path::remove_dots(res, /*remove_dot_dot=*/true);
  return std::string(res);
}

}  // anonymous namespace

bool ExecuteAction(int argc,
                   const char** argv,
                   ActionFactory make_iwyu_action) {
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

  std::vector<const char*> extra_args;
  // Add -fsyntax-only to avoid code generation, unless user asked for
  // preprocessing-only.
  if (!HasPreprocessOnlyArgs(args)) {
    extra_args.push_back("-fsyntax-only");
    extra_args.push_back("-Qunused-arguments");
  }

  std::string iwyu_executable_path = GetExecutablePath(argv[0]);
  if (!HasArg(args, "-resource-dir") && !HasArgPrefix(args, "-resource-dir=")) {
    // If user didn't specify something explicit, compute a resource dir based
    // on configured CMake variables.
    std::string resource_dir = ComputeCustomResourceDir(iwyu_executable_path);
    if (!resource_dir.empty()) {
      extra_args.push_back("-resource-dir");
      extra_args.push_back(SaveStringInSet(SavedStrings, resource_dir));
    }
  }

  // If there is no -- in the args, the extra_pos will be args.end() and insert
  // will append to the back of the args sequence.
  auto extra_pos =
      llvm::find_if(args, [](StringRef arg) { return arg == "--"; });
  args.insert(extra_pos, extra_args.begin(), extra_args.end());

  IntrusiveRefCntPtr<FileSystem> fs = llvm::vfs::getRealFileSystem();
  DiagnosticOptions diag_opts;
  IntrusiveRefCntPtr<DiagnosticsEngine> diagnostics =
      CompilerInstance::createDiagnostics(*fs, diag_opts);

  // The Driver constructor sets the resource dir implicitly based on path,
  // which may then be overwritten by BuildCompilation based on any
  // -resource-dir argument from above.
  Driver driver(iwyu_executable_path, getDefaultTargetTriple(), *diagnostics);
  driver.setTitle("include what you use");

  // Build a compilation, get the job list and filter out irrelevant jobs.
  unique_ptr<Compilation> compilation(driver.BuildCompilation(args));
  if (!compilation)
    return false;

  // This diagnostic switch is handled and executed inside BuildCompilation.
  // Exit immediately so we don't print errors.
  if (HasArg(args, "-print-resource-dir")) {
    return false;
  }

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
  unique_ptr<CompilerInstance> compiler(
      new CompilerInstance(std::move(invocation)));
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
    case Action::PrecompileJobClass:
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
