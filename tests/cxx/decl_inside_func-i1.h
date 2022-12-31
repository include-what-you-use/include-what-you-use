//===--- decl_inside_func-i1.h - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Reduced/derived/copied from Quill 1.6.3:
// https://github.com/odygrd/quill/blob/v1.6.3/quill/include/quill/PatternFormatter.h#L35

// This macro defines a lambda containing a function-local declaration of a
// union X, which is cleverly returned from the lambda.
//
// Declarations inside a function are only directly visible from the function
// itself, so any uses of X should be ignored -- the only way to get in any
// contact with X is to use MACRO, so this header will be desired already.

#define MACRO(str)                    \
  [] {                                \
    union X {                         \
      static constexpr auto value() { \
        return str;                   \
      }                               \
    };                                \
    return X{};                       \
  }()

/**** IWYU_SUMMARY

(tests/cxx/decl_inside_func-i1.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
