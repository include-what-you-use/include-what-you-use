//===--- member_expr-d1.h - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#define CALL_METHOD Method()
#define IC ic
#define IC_CALL_METHOD  (ic).Method()
#define IC_CALL_METHOD_MULTILINE (ic). \
    Method()
#define DOT_METHOD .Method()
#define IC_DOT ic.
