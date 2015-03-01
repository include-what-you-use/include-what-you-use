//===--- catch-logex.h - test input file for iwyu -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_CATCH_LOGEX_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_CATCH_LOGEX_H_

template<class ExceptionType>
inline void LogException(const ExceptionType&) {
  // log exception
}

#endif // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_CATCH_LOGEX_H_
