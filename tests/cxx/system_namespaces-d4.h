//===--- system_namespaces-d4.h - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

namespace __gnu_cxx {
template <typename, typename>
class __normal_iterator {
 public:
  int& operator*();
  __normal_iterator& operator++();
};

template <typename T1, typename T2>
bool operator!=(__normal_iterator<T1, T2>, __normal_iterator<T1, T2>);
}  // namespace __gnu_cxx

namespace std {
template <typename T, decltype(sizeof(int)) Size>
struct array {
  using iterator = __gnu_cxx::__normal_iterator<int*, array<T, Size>>;

  iterator begin();
  iterator end();
};
}  // namespace std

struct ArrayDerived : std::array<int, 1> {};
