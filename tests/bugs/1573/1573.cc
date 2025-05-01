// IWYU_XFAIL

namespace foo {
struct Bar;
}

struct foo::Bar {
  void frombulate();
};

/**** IWYU_SUMMARY

(tests/bugs/1573/1573.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
