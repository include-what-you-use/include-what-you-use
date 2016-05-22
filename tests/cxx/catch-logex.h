//===--- catch-logex.h - test input file for iwyu -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_CATCH_LOGEX_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_CATCH_LOGEX_H_

template<class ExceptionType>
inline void LogException(const ExceptionType&) {
  // log exception
}

#endif // INCLUDE_WHAT_YOU_USE_TESTS_CXX_CATCH_LOGEX_H_
