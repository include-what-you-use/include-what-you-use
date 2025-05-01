#pragma once

#include <vector>

template<typename T>
class MyVector {
 public:
  using iterator = typename std::vector<T>::iterator;

  iterator begin() { return data_.begin(); }
  iterator end() { return data_.end(); }

 private:
  std::vector<T> data_;
};
