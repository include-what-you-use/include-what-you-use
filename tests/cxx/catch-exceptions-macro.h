//===--- catch-exceptions-macro.h - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_CATCH_EXCEPTIONS_MACRO_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_CATCH_EXCEPTIONS_MACRO_H_

// Named catch
#define CHECK_THROW_1(E) \
do { \
    try { \
        ; \
    } catch (const E& ex) { \
        (void)ex; \
    } \
} while (0)

// Unnamed catch
#define CHECK_THROW_2(E) \
do { \
    try { \
        ; \
    } catch (const E&) { \
        ; \
    } \
} while (0)

#endif // INCLUDE_WHAT_YOU_USE_TESTS_CXX_CATCH_EXCEPTIONS_MACRO_H_
