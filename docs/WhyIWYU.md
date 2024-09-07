# Why include what you use? #

Are there any concrete benefits to a strict include-what-you-use policy? We like
to think so.


## Faster compiles ##

Every .h file you bring in when compiling a source file lengthens the time to
compile, as the bytes have to be read, preprocessed, and parsed.  If you're not
actually using a .h file, you remove that cost.  With template code, where
entire instantiations have to be in .h files, this can be hundreds of thousands
of bytes of code.  In one case at Google, running include-what-you-use over a
.cc file improved its compile time by 30%.

Here, the main benefit of include-what-you-use comes from the flip side: "don't
include what you don't use."


## Fewer recompiles ##

Many build tools, such as `make`, provide a mechanism for automatically figuring
out what .h files a .cc file depends on.  These mechanisms typically look at
`#include` lines.  When unnecessary `#includes` are listed, the build system is
more likely to recompile in cases where it's not necessary.

Again, the main advantage here is from "don't include what you don't use."


## Allow refactoring ##

Suppose you refactor `foo.h` so it no longer uses vectors.  You'd like to remove
`#include <vector>` from `foo.h`, to reduce compile time -- template class files
such as `vector` can include a lot of code.  But can you?  In theory yes, but in
practice maybe not: some other file may be #including you and using vectors, and
depending (probably unknowingly) on your `#include <vector>` to compile.  Your
refactor could break code far away from you.

This is most compelling for a very large codebase (such as Google's).  In a
small codebase, it's practical to just compile everything after a refactor like
this, and clean up any errors you see.  When your codebase contains hundreds of
thousands of source files, identifying and cleaning up the errors can be a
project in itself.  In practice, people are likely to just leave the `#include
<vector>` line in there, even though it's unnecessary.

Here, it's the actual 'include what you use' policy that saves the day.  If
everyone who uses vector is #including `<vector>` themselves, then you can
remove `<vector>` without fear of breaking anything.


## Self-documentation ##

When you can trust the `#include` lines to accurately reflect what is used in
the file, you can use them to help you understand the code.  Looking at them, in
itself, can help you understand what this file needs in order to do its work.
If you use the optional 'commenting' feature of `fix_includes.py`, you can see
what symbols -- what functions and classes -- are used by this code.  It's like
a pared-down version of doxygen markup, but totally automated and present where
the code is (rather than in a separate web browser).

The 'commented' `#include` lines can also make it simpler to match function
calls and classes to the files that define them, without depending on a
particular IDE.

(The downside, of course, is the comments can get out of date as the code
changes, so unless you run IWYU often, you still have to take the comments with
a grain of salt.  Nothing is free. :-) )


## Dependency cutting ##

Again, this makes the most sense for large code-bases.  Suppose your binaries
are larger than you would expect, and upon closer examination use symbols that
seem totally irrelevant.  Where do they come from?  Why are they there?  With
include-what-you-use, you can easily determine this by seeing who includes the
files that define these symbols: those includers, and those alone, are
responsible for the use.

Once you know where a symbol is used in your binary, you can see how practical
it is to remove that use, perhaps by breaking up the relevant .h files into two
parts, and fixing up all callers.  Again it's IWYU to the rescue: with
include-what-you-use, figuring out the callers that need fixing is easy.


## Why forward-declare? ##

Include-what-you-use tries very hard to figure out when a forward-declare can be
used instead of an `#include` (IWYU would be about 90% less code if it didn't
bother with trying to forward-declare).

The reason for this is simple: if you can replace an `#include` by a
forward-declare, you reduce the code size, speeding up compiles as described
above.  You also make it easier to break dependencies: not only do you not
depend on that header file, you no longer depend on everything it brings in.

There's a cost to forward-declaring as well: you lose the documentation features
mentioned above, that come with `#include` lines.  (A future version of IWYU may
mitigate this problem.)  And if a class changes -- for instance, it adds a new
default template argument -- you need to change many callsites, not just one.
It is also easier to accidentally violate the [One Definition
Rule](http://en.wikipedia.org/wiki/One_Definition_Rule) when all you expose is
the name of a class (via a forward declare) rather than the full definition (via
an `#include`).

One compromise approach is to use 'forwarding headers', such as `<iosfwd>`.
These forwarding headers could have comments saying where the definition of each
forward-declared class is.  Include-what-you-use does not currently support
forwarding headers, but may in the future.

Since some coding standards have taken to [discourage forward
declarations](https://google.github.io/styleguide/cppguide.html#Forward_Declarations),
IWYU has grown a `--no_fwd_decls` mode to embrace this alternative
strategy. Where IWYU's default behavior is to minimize the number of include
directives, IWYU with `--no_fwd_decls` will attempt to minimize the number of
times each type is redeclared. The result is that include directives will always
be preferred over local forward declarations, even if it means including a
header just for a name-only type declaration.

We still think IWYU's normal policy is preferable for all the reasons above, but
if your codebase has a no-forward-declare policy, so does IWYU.
