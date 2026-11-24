//===--- specialization_use_alternating-defs.h - test input ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

template <class... T> struct Sink {};
template <class T> struct Box1 { template <class> struct Nested {}; };
template <class T> struct Box2 { template <class> struct Nested {}; };
template <class T, template <class> class N> struct ATQ {};
struct Payload {};
