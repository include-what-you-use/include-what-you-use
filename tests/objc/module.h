@import Foo;
#import <Foo/Foo.h>
@import Foo;
#import <stdio.h>

@interface Test
@end

/**** iwyu_summary

(tests/objc/module.h has correct #includes/fwd-decls)

***** iwyu_summary */
