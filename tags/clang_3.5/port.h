//===--- port.h - OS/cpu specific stuff for include-what-you-use ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// Source file for architecture-specific logic.

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_PORT_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_PORT_H_

#include <stdlib.h>  // for abort
#include <iostream>
#include "llvm/Support/Compiler.h"

// Portable stub for Clang's __has_feature.
#ifndef __has_feature
# define __has_feature(x) 0
#endif

// Portable override keyword.
// Use to mark virtual methods as overriding a base class method,
// compiler will complain if method does not exist in base class.
#if (defined(_MSC_VER) || __has_feature(cxx_override_control))
#define IWYU_OVERRIDE override
#else
#define IWYU_OVERRIDE
#endif

// Count of statically allocated array.
#define IWYU_ARRAYSIZE(arr) sizeof(arr) / sizeof(*arr)

namespace include_what_you_use {

// Helper class that allows programmers to log extra information in CHECK_s.
class FatalMessageEmitter {
 public:
  FatalMessageEmitter(const char* file, int line, const char* message) {
    stream() << file << ":" << line << ": Assertion failed: " << message;
  }
  LLVM_ATTRIBUTE_NORETURN ~FatalMessageEmitter() {
    stream() << ::std::endl;
    ::abort();
#ifdef LLVM_BUILTIN_UNREACHABLE
    // Windows systems and possibly others don't declare abort() to be noreturn,
    // so use the unreachable builtin to avoid a Clang self-host warning.
    LLVM_BUILTIN_UNREACHABLE;
#endif
  }
  ::std::ostream& stream() { return ::std::cerr; }
};

// Helper class that allows an ostream to 'appear' as a void expression.
class OstreamVoidifier {
 public:
  // This has to be an operator with a precedence lower than << but
  // higher than ?:
  void operator&(::std::ostream&) {}
};

}  // namespace include_what_you_use

// Usage: CHECK_(condition) << extra << information;
// The file, line, condition and extra information will be printed to cerr,
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

#if defined(_WIN32)

#define snprintf _snprintf

#define NOMINMAX  // Prevent Windows headers from redefining min/max.
#include "Shlwapi.h"  // for PathMatchSpec

// This undef is necessary to prevent conflicts between llvm
// and Windows headers.
// objbase.h has #define interface struct.
#undef interface

inline bool GlobMatchesPath(const char *glob, const char *path) {
  return PathMatchSpec(path, glob);
}

#else  // #if defined(_WIN32)

#include <fnmatch.h>

inline bool GlobMatchesPath(const char *glob, const char *path) {
  return fnmatch(glob, path, FNM_PATHNAME) == 0;
}

#endif  // #if defined(_WIN32)

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_PORT_H_
