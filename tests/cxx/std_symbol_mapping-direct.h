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

template <typename>
struct char_traits;
template <>
struct char_traits<char> {};

template <typename T, typename = char_traits<T>>
class basic_ostream;
template <typename, typename>
class basic_ostream {};

template <typename T, typename = char_traits<T>>
class basic_spanstream;
template <typename, typename>
class basic_spanstream {};
// wspanstream should not be considered as providing typedef despite being
// placed after basic_spanstream definition (like in libstdc++).
using wspanstream = basic_spanstream<wchar_t>;

template <typename T, typename = char_traits<T>>
class basic_fstream;
using fstream = basic_fstream<char>;

using intmax_t = long long;

template <intmax_t, intmax_t = 1>
class ratio;

namespace chrono {
template <typename, typename = ratio<1>>
class duration {};

using seconds = duration<long long>;
}  // namespace chrono

inline namespace literals {
inline namespace chrono_literals {
// This approximately resembles how operator""s is defined in libstdc++.
template <char...>
chrono::seconds operator""s();
}  // namespace chrono_literals
}  // namespace literals
}
