#include <vector>

template <typename T>
class Vector : private std::vector<T> {
 public:
  using Base = std::vector<T>;

  using Base::Base;
  using Base::begin;
  using Base::end;
};
