//===--- lambda_fwd_decl.cc - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This test ensures that we do not add forward-declarations for lambdas.
// The use of the lambda in add() registers as a use of the generated anonymous
// functor type. Since it's defined in the same file, IWYU thinks a forward-decl
// at the top would be useful.
//
// All this is obviously unnecessary -- lambdas have no name and all
// lambda declarations are definitions, visible at point of use.

template<class F>
int eval(F f, int arg) {
  return f(arg);
}

int add(int value, int addend) {
  return eval([&](int v) { return v + addend; },
              value);
}

/**** IWYU_SUMMARY

(tests/cxx/lambda_fwd_decl.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
