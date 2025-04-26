#define MACRO() MacroClass()

class MacroClass {
public:
  template <typename T> T func() { return T(); }
};
