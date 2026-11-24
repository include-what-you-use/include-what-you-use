//===--- specialization_use_alternating-aliases.h - test input ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "tests/cxx/specialization_use_alternating-defs.h"

// Each shared leaf reports two different explicit instantiations. Flattening
// repeated leaves cannot be made cheap by merging adjacent identical uses.
template <class T>
using Leaf = Sink<ATQ<Box1<T>, Box1<T>::template Nested>,
                  ATQ<Box2<T>, Box2<T>::template Nested>>;
template <class T> using Dup = Sink<T, T>;
template <class T> using D0 = Leaf<T>;
template <class T> using D1 = Dup<D0<T>>;
template <class T> using D2 = Dup<D1<T>>;
template <class T> using D3 = Dup<D2<T>>;
template <class T> using D4 = Dup<D3<T>>;
template <class T> using D5 = Dup<D4<T>>;
template <class T> using D6 = Dup<D5<T>>;
template <class T> using D7 = Dup<D6<T>>;
template <class T> using D8 = Dup<D7<T>>;
template <class T> using D9 = Dup<D8<T>>;
template <class T> using D10 = Dup<D9<T>>;
template <class T> using D11 = Dup<D10<T>>;
template <class T> using D12 = Dup<D11<T>>;
template <class T> using D13 = Dup<D12<T>>;
template <class T> using D14 = Dup<D13<T>>;
template <class T> using D15 = Dup<D14<T>>;
template <class T> using D16 = Dup<D15<T>>;
template <class T> using D17 = Dup<D16<T>>;
template <class T> using D18 = Dup<D17<T>>;
template <class T> using D19 = Dup<D18<T>>;
template <class T> using D20 = Dup<D19<T>>;

// Keep the default shallow enough for comparison with unoptimized traversal.
#ifndef IWYU_ALTERNATING_DEPTH
#define IWYU_ALTERNATING_DEPTH D4
#endif
template <class T> struct Holder { IWYU_ALTERNATING_DEPTH<T> value; };
