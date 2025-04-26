#include <memory>

template <typename T>
class Template {
 public:
  typedef T Type;
  typedef std::shared_ptr<Type> TypeSPtr;

  void foo() {
    TypeSPtr obj = std::make_shared<Type>();
  }
};
