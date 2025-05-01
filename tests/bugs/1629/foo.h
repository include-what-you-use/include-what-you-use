#include <vector>

template <typename T, typename Alloc = typename std::vector<T>::allocator_type>
class Vector final : private std::vector<T, Alloc> {
 public:
  Vector() = default;
};
