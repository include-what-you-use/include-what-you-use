namespace FooNs {
  class FooClass {};
};

namespace BarNs {
  using namespace FooNs;

  class BarClass {
  };
}
