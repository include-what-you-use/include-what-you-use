
// macro_and_template-d1.h
#define MACRO() MacroClass()

class MacroClass {
public:
  template <typename T> T func() { return T(); }
};
