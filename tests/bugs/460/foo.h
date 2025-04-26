#include <vector>
#include <list>

class bar {
 public:
  using MyVec = std::vector<int>;
};

namespace bar2 {
using MyList = std::list<int>;
}
