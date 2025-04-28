template <typename> struct enum_data {
  static const int staticVar;
  virtual void var() const;
};
template <typename E> const int enum_data<E>::staticVar = 0;
template <typename E> void enum_data<E>::var() const { (void)staticVar; }

/**** IWYU_SUMMARY

(tests/bugs/1181/1181.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
