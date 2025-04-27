#include <memory>
class fooB;

std::unique_ptr<fooB> getPointerFooB() {
  return std::make_unique<fooB>();
}
