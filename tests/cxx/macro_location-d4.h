//===--- macro_location-d4.h - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This file contains negative cases for the forward-declares used for
// use-attribution hints when expanding macros.

#include "tests/cxx/macro_location-i5.h" // for UNNAMED_TYPE_IN_MACRO

// A class declared in this file and used via a macro defined in this file
// should not be seen by IWYU in files expanding the macro.
class Logger {
 public:
  void Log(int level, const char* msg);
};

// A forward-declaration inside a macro should not count as a use-attribution
// hint and should not be seen by IWYU in files expanding other macros.
#define UNRELATED_FORWARD_DECLARE class Logger;

#define LOG_INFO(x) \
  Logger().Log(0, x);

// A class declared and used in this macro should not be seen by IWYU in files
// expanding the macro. This is similar to USE_CLASS in macro_location-d2.h, but
// doesn't involve macro arg concatentation.
// (And yes, this code is broken in so many ways, it's only to put the test case
// in a vaguely real-life scenario)
#define DECLARE_AND_USE_CLASS(y)     \
  class ScopedLogger {               \
   public:                           \
    ScopedLogger() { LOG_INFO(y); }  \
    ~ScopedLogger() { LOG_INFO(y); } \
  };                                 \
  ScopedLogger lll;
