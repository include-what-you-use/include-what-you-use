//===--- iwyu_port.h - OS/cpu specific stuff for include-what-you-use -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// Source file for architecture-specific logic.

#ifndef INCLUDE_WHAT_YOU_USE_PORT_H_
#define INCLUDE_WHAT_YOU_USE_PORT_H_

#include <cstdlib>   // for abort

#include "llvm/Support/Compiler.h"
#include "llvm/Support/raw_ostream.h"

// Count of statically allocated array.
#define IWYU_ARRAYSIZE(arr) sizeof(arr) / sizeof(*arr)

namespace include_what_you_use {

// Helper class that allows programmers to log extra information in CHECK_s.
class FatalMessageEmitter {
 public:
  FatalMessageEmitter(const char* file, int line, const char* message) {
    stream() << file << ":" << line << ": Assertion failed: " << message;
  }
  [[noreturn]] ~FatalMessageEmitter() {
    stream() << "\n";
    ::abort();
#ifdef LLVM_BUILTIN_UNREACHABLE
    // Windows systems and possibly others don't declare abort() to be noreturn,
    // so use the unreachable builtin to avoid a Clang self-host warning.
    LLVM_BUILTIN_UNREACHABLE;
#endif
  }
  llvm::raw_ostream& stream() {
    return llvm::errs();
  }
};

// Helper class that allows an ostream to 'appear' as a void expression.
class OstreamVoidifier {
 public:
  // This has to be an operator with a precedence lower than << but
  // higher than ?:
  void operator&(llvm::raw_ostream&) {}
};

}  // namespace include_what_you_use

// Usage: CHECK_(condition) << extra << information;
// The file, line, condition and extra information will be printed to stderr,
// then the program will abort.
#define CHECK_(x)  (x) ? (void)0 : \
  ::include_what_you_use::OstreamVoidifier() & \
  ::include_what_you_use::FatalMessageEmitter( \
      __FILE__, __LINE__, #x).stream()
// Instead of CHECK_(false && "message") use CHECK_UNREACHABLE_("message").
#define CHECK_UNREACHABLE_(message) \
  ::include_what_you_use::OstreamVoidifier() & \
  ::include_what_you_use::FatalMessageEmitter( \
      __FILE__, __LINE__, message).stream()

bool GlobMatchesPath(const char *glob, const char *path);

#endif  // INCLUDE_WHAT_YOU_USE_PORT_H_
