//===--- std_symbol_mapping-direct.h - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

namespace std {
typedef decltype(sizeof(int)) size_t;

template <typename, typename>
struct pair;

template <typename T1, typename T2, typename U1, typename U2>
bool operator==(const pair<T1, T2>&, const pair<U1, U2>&);

template <typename>
class function;
template <typename Ret, typename... Args>
class function<Ret(Args...)>;

template <typename T, size_t N>
struct array;

template <typename T, size_t N>
bool operator==(const array<T, N>&, const array<T, N>&);

template <size_t I, typename T1, typename T2>
void get(pair<T1, T2>&) noexcept;
template <size_t I, typename T1, typename T2>
void get(pair<T1, T2>&&) noexcept;
template <typename T1, typename T2>
const T1& get(const pair<T1, T2>& p) noexcept;
template <typename T2, typename T1>
const T2& get(const pair<T1, T2>& p) noexcept;

template <size_t I, typename T, size_t N>
const T&& get(const array<T, N>&&) noexcept;

template <typename T>
__remove_reference_t(T)&& move(T&&) noexcept;

template <class I, class O>
O move(I, I, O);

template <typename T>
void swap(T& x, T& y);
template <typename T, size_t N>
void swap(T (&x)[N], T (&y)[N]);
template <typename Ret, typename... Args>
void swap(function<Ret(Args...)>&, function<Ret(Args...)>&);

template <typename T>
class valarray {
 public:
  using value_type = T;
};

template <typename T>
valarray<T> pow(const valarray<T>&, const typename valarray<T>::value_type&);

float pow(float, float);
double pow(double, double);
long double pow(long double, long double);
}
