template <typename T>
struct IntegerWrapper {};

template <typename T>
struct NumGetter {
  int GetNum() {
    return IntegerWrapper<T>::num;
  }
};
