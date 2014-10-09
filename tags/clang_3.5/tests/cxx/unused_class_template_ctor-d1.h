//===--- unused_class_template_ctor-d1.h - test input file for iwyu -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_UNUSED_CLASS_TEMPLATE_CTOR_D1_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_UNUSED_CLASS_TEMPLATE_CTOR_D1_H_

template <class T1, class T2>
class pair {
 private:
  T1 first_;
  T2 second_;
 public:
  // If T2 is a reference type, 2-argument constructor is valid but 1-argument
  // constructor cannot be used.  Instantiating 1-argument constructor causes
  //
  // error: constructor for 'pair<int &, int &>' must explicitly initialize
  // the reference member 'second_'
  explicit pair(T1 t1) : first_(t1) {}
  pair(T1 t1, T2 t2) : first_(t1), second_(t2) {}
};

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_UNUSED_CLASS_TEMPLATE_CTOR_D1_H_
