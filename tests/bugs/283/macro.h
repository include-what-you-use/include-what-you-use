// macro.h
#define MACRO_A(domain) \
  void myfunc() { \
    static ns::Class s_var(ns::getmyclass##domain()); \
  }
