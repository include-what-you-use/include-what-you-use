template <>
struct IntegerWrapper<Foo> {
  static constexpr int num = Foo::num;
};
