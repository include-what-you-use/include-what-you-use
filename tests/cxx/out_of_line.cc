//===--- out_of_line.cc - test input file for iwyu ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "out_of_line-dep.h"

template <class T, class U = char>
struct Class {
  using Type = T;

  void MethodIn() { Dependent(T()); }
  void MethodOut();

  static void MethodStaticIn() { Dependent(T()); }
  static void MethodStaticOut();
};

template <class T, class U>
void Class<T, U>::MethodOut() {
  Dependent(T());
}

template <class T, class U>
void Class<T, U>::MethodStaticOut() {
  Dependent(T());
}

template <>
void Class<int, int>::MethodOut() {
  // IWYU: Dependent is...*out_of_line-dep-int.h
  Dependent(Type());
}


int main()
{
    //Inline template method
    Class<int> c;
    // IWYU: Dependent is...*out_of_line-dep-int.h
    c.MethodIn();

    //Out-of-line template method
    Class<int> c2;
    // IWYU: Dependent is...*out_of_line-dep-int.h
    c2.MethodOut();

    //A explicit specialization of MethodOut(). It does not
    //require Dependent(int) here, because it must be already
    //available at the definition.
    Class<int, int> c3;
    c3.MethodOut();

    //Inline static method
    // IWYU: Dependent is...*out_of_line-dep-int.h
    Class<int>::MethodStaticIn();

    //Out-of-line static method
    // IWYU: Dependent is...*out_of_line-dep-int.h
    Class<int>::MethodStaticOut();
}

/**** IWYU_SUMMARY

tests/cxx/out_of_line.cc should add these lines:
#include "out_of_line-dep-int.h"

tests/cxx/out_of_line.cc should remove these lines:
- #include "out_of_line-dep.h"  // lines XX-XX

The full include-list for tests/cxx/out_of_line.cc:
#include "out_of_line-dep-int.h"  // for Dependent

***** IWYU_SUMMARY */
