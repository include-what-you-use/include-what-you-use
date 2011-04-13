//===--- badinc-i3.h - test input file for iwyu ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_I3_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_I3_H_

// I only forward-declare things from this file.

class I3_ForwardDeclareClass {
 public:
  int a;
};

struct I3_ForwardDeclareStruct {
  int a;
};

template<typename A>
struct I3_SimpleForwardDeclareTemplateStruct {
  A a;
};

template<typename A, int B, char C>
struct I3_ForwardDeclareTemplateStruct {
  A a;
};

class I3_UnusedClass {   // unused by badinc.cc
 public:
  int a;
};

namespace i3_ns1 {

namespace {
struct I3_UnnamedNamespaceStruct {};
}  // namespace

namespace i3_ns2 {
namespace i3_ns3 {

struct I3_ForwardDeclareNamespaceStruct {
  int a;
};

template<typename A, int B>
struct I3_ForwardDeclareNamespaceTemplateStruct {
  A a;
};

}
}
}

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_I3_H_
