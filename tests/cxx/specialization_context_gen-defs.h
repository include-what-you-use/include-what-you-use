//===--- specialization_context_gen-defs.h - test input -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

// Uses Helper without including its header, so that the uses of Helper in
// the specializations below are attributed to their users.

template <class...>
struct Sink {};

template <int>
struct Arr {};

template <class T>
T Make();

template <class T>
using Q = Sink<decltype(Helper(), Make<T>())>;

// Traversing this variable template makes the nodes of its pattern, among
// them its callee Helper, ignored for the rest of the scan.
template <class T>
constexpr int kV = Helper() + sizeof(T);

// Reaches U, then the variable template, then U again.
template <class U>
using GenThen = Sink<U, Arr<kV<int>>, U>;

template <class T>
struct Holder {
  GenThen<Q<T>> value;
};
