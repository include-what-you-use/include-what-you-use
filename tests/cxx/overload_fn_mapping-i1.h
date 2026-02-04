//===--- overload_fn_mapping-i1.h - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

class A {};
class B {};

namespace ns {
class Class {};

using ::B;

void Fn();
}  // namespace ns

using ConstLong = const long;

int Fn();
int Fn(int);
int Fn(char);
int Fn(int, int);
int Fn(int, char);
int Fn(int*);
int Fn(double&);
int Fn(unsigned long&&);
int Fn(const double&);
int Fn(const int*&);
int Fn(char* const&);
int Fn(int());
int Fn(char, int);
int Fn(const double, int* const);
int Fn(A);
int Fn(ns::Class);
int Fn(float, int, ...);
int Fn(float, int = 0);
int Fn(ns::B);
int Fn(ConstLong);
int Fn(unsigned const[][5][7]);

int FnTakesAll(...);
int FnTakesAll(int);

namespace ns {
int FnFromNs();
int FnFromNs(Class);

inline namespace inl {
class C {};
int FnFromNs(const C&);
}  // namespace inl
}  // namespace ns

bool operator==(const A&, const A&);
bool operator==(const B&, const B&);
bool operator==(A, B);
bool operator==(B, A);

template <typename>
class Tpl {};

template <typename T, typename U>
int TplFn(T, U);

template <typename T>
int TplFn(auto, T*);

template <typename T>
int TplFn(Tpl<T>);

// Something like std::swap for arrays.
template <class T, int N>
void TplFn(T (&x)[N]);
