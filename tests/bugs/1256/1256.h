struct results;

struct funcs {
  // No diagnostic expected
  results (*get_results)();
};

/**** IWYU_SUMMARY

(tests/bugs/1256/1256.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
