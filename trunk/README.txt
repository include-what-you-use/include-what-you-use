--------------------------------------------------------------------------------
 Include What You Use
--------------------------------------------------------------------------------

This README was generated from the Wiki contents at
http://code.google.com/p/include-what-you-use/w/ on 2014-01-21 20:11:08 UTC.


= Instructions for Users  =

"Include what you use" means this: for every symbol (type, function, variable,
or macro) that you use in foo.cc (or foo.cpp), either foo.cc or foo.h should
#include a .h file that exports the declaration of that symbol. (Similarly, for
foo_test.cc, either foo_test.cc or foo.h should do the #including.)  Obviously
symbols defined in foo.cc itself are excluded from this requirement.

This puts us in a state where every file includes the headers it needs to
declare the symbols that it uses.  When every file includes what it uses, then
it is possible to edit any file and remove unused headers, without fear of
accidentally breaking the upwards dependencies of that file.  It also becomes
easy to automatically track and update dependencies in the source code.


== CAVEAT ==

This is alpha quality software -- at best (as of February 2011).  It was written
to work specifically in the Google source tree, and may make assumptions, or
have gaps, that are immediately and embarrassingly evident in other types of
code.  For instance, we only run this on C++ code, not C or Objective C.  Even
for Google code, the tool still makes a lot of mistakes.

While we work to get IWYU quality up, we will be stinting new features, and will
prioritize reported bugs along with the many existing, known bugs.  The best
chance of getting a problem fixed is to submit a patch that fixes it (along with
a unittest case that verifies the fix)!


== How to Build ==

Include-what-you-use makes heavy use of Clang internals, and will occasionally
break when Clang is updated. See the include-what-you-use Makefile for
instructions on how to keep them in sync.

IWYU, like Clang, does not yet handle some of the non-standard constructs in
Microsoft's STL headers. A discussion on how to use MinGW or Cygwin headers with
IWYU is available on the mailing list.

We support two build configurations: out-of-tree and in-tree.


=== Building out-of-tree ===

In an out-of-tree configuration, we assume you already have compiled LLVM and
Clang headers and libs somewhere on your filesystem, such as via the libclang-
dev package. Out-of-tree builds are only supported with CMake (patches very
welcome for the Make system).

  * Create a directory for IWYU development, e.g. iwyu-trunk
  * Get the IWYU source code, either from a published tarball or directly from
svn

    # Unpack tarball
    iwyu-trunk$ tar xfz include-what-you-use-<version>.tar.gz

    # or checkout from SVN
    iwyu-trunk$ svn co http://include-what-you-use.googlecode.com/svn/trunk/
include-what-you-use

  * Create a build root

    iwyu-trunk$ mkdir build && cd build

  * Run CMake and specify the location of LLVM/Clang prebuilts

    # This example uses the Makefile generator,
    # but anything should work.
    iwyu-trunk/build$ cmake -G "Unix Makefiles" -DLLVM_PATH=/usr/lib/llvm-3.4
../include-what-you-use

  * Once CMake has generated a build system, you can invoke it directly from
build, e.g.

    iwyu-trunk/build$ make


This configuration is more useful if you want to get IWYU up and running quickly
without building Clang and LLVM from scratch.


=== Building in-tree ===

You will need the Clang and LLVM trees on your system, such as by checking out
their SVN trees (but don't configure or build before you've done the following.)

  * Put include-what-you-use in the source tree. Either download the include-
what-you-use tarball and unpack it your /path/to/llvm/tools/clang/tools
directory, or get the project directly from svn:

    # Unpack tarball
    llvm/tools/clang/tools$ tar xfz include-what-you-use-<version>.tar.gz

    # or checkout from SVN
    llvm/tools/clang/tools$ svn co http://include-what-you-
use.googlecode.com/svn/trunk/ include-what-you-use

  * Edit tools/clang/tools/Makefile and add include-what-you-use to the DIRS
variable
  * Edit tools/clang/tools/CMakeLists.txt and add_subdirectory(include-what-you-
use)
  * Once this is done, IWYU is recognized and picked up by both autoconf and
CMake workflows as described in the Clang Getting Started guide

This configuration is more useful if you're actively developing IWYU against
Clang trunk.


== How to Run ==

The easiest way to run IWYU over your codebase is to run

  make -k CXX=/path/to/llvm/Debug+Asserts/bin/include-what-you-use

or

  make -k CXX=/path/to/llvm/Release/bin/include-what-you-use

(include-what-you-use always exits with an error code, so the build system knows
it didn't build a .o file.  Hence the need for -k.)

We also include, in this directory, a tool that automatically fixes up your
source files based on the iwyu recommendations.  This is also alpha-quality
software!  Here's how to use it (requires python):

  make -k CXX=/path/to/llvm/Debug+Asserts/bin/include-what-you-use >
/tmp/iwyu.out
  python fix_includes.py < /tmp/iwyu.out

If you don't like the way fix_includes.py munges your #include lines, you can
control its behavior via flags. fix_includes.py --help will give a full list,
but these are some common ones:

  * -b: Put blank lines between system and Google #includes
  * --nocomments: Don't add the 'why' comments next to #includes

WARNING: include-what-you-use only analyzes .cc (or .cpp) files built by make,
along with their corresponding .h files.  If your project has a .h file with no
corresponding .cc file, iwyu will ignore it. include-what-you-use supports the
AddGlobToReportIWYUViolationsFor() function which can be used to indicate other
files to analyze, but it's not currently exposed to the user in any way.


== How to Correct IWYU Mistakes ==

  * If fix_includes.py has removed an #include you actually need, add it back in
with the comment '// IWYU pragma: keep' at the end of the #include line.  Note
that the comment is case-sensitive.
  * If fix_includes has added an #include you don't need, just take it out.  We
hope to come up with a more permanent way of fixing later.
  * If fix_includes has wrongly added or removed a forward-declare, just fix it
up manually.
  * If fix_includes has suggested a private header file (such as
<bits/stl_vector.h>) instead of the proper public header file (<vector>), you
can fix this by inserting a specially crafted comment near top of the private
file (assuming you can write to it): '// IWYU pragma: private, include
"the/public/file.h"'.

All current IWYU pragmas (as of July 2012) are described in [IWYUPragmas].


= Instructions for Developers =

== Submitting Patches ==

We're still working this part out.  For now, you can create patches against svn-
head and submit them as new issues.  Probably, we'll move to a scheme where
people can submit patches directly to the SVN repository.


== Running the Tests ==

If fixing a bug in clang, please add a test to the test suite!  You
can create a file called whatever.cc (_not_ .cpp), and, if necessary,
whatever.h, and whatever-<extension>.h.  You may be able to get away without
adding any .h files, and just #including direct.h -- see, for instance,
tests/remove_fwd_decl_when_including.cc.

To run the iwyu tests, run

  python run_iwyu_tests.py

It runs one test for each .cc file in the tests/ directory.  (We have additional
tests in more_tests/, but have not yet gotten the testing framework set up for
those tests.)  The output can be a bit hard to read, but if a test fails, the
reason why will be listed after the
ERROR:root:Test failed for xxx line.

When fixing fix_includes.py, add a test case to fix_includes_test.py and run

  python fix_includes_test.py


== Debugging ==

It's possible to run include-what-you-use in gdb, to debug that way.
Another useful tool -- especially in combination with gdb -- is to get
the verbose include-what-you-use output.  See iwyu_output.h for a
description of the verbose levels.  Level 7 is very verbose -- it
dumps basically the entire AST as it's being traversed, along with
iwyu decisions made as it goes -- but very useful for that:

  env IWYU_VERBOSE=7 make -k CXX=/path/to/llvm/Debug+Asserts/bin/include-what-
you-use 2>&1 > /tmp/iwyu.verbose


== A Quick Tour of the Codebase ==

The codebase is strewn with TODOs of known problems, and also language
constructs that aren't adequately tested yet.  So there's plenty to do!  Here's
a brief guide through the codebase:

  * iwyu.cc: the main file, it includes the logic for deciding when a symbol has
been 'used', and whether it's a full use (definition required) or forward-
declare use (only a declaration required).  It also includes the logic for
following uses through template instantiations.
  * iwyu_driver.cc: responsible for creating and configuring a Clang compiler
from command-line arguments.
  * iwyu_output.cc: the file that translates from 'uses' into iwyu violations.
This has the logic for deciding if a use is covered by an existing #include (or
is a built-in).  It also, as the name suggests, prints the iwyu output.
  * iwyu_preprocessor.cc: handles the preprocessor directives, the #includes and
#ifdefs, to construct the existing include-tree.  This is obviously essential
for include-what-you-use analysis.  This file also handles the iwyu pragma-
comments.
  * iwyu_include_picker.cc: this finds canonical #includes, handling
private->public mappings (like bits/stl_vector.h -> vector) and symbols with
multiple possible #includes (like NULL). Mappings are maintained in a set of
.imp files separately, for easier per-platform/-toolset customization.
  * iwyu_cache.cc: holds the cache of instantiated templates (may hold other
cached info later).  This is data that is expensive to compute and may be used
more than once.
  * iwyu_globals.cc: holds various global variables.  We used to think globals
were bad, until we saw how much having this file simplified the code...
  * iwyu_*_util(s).h and .cc: utility functions of various types.  The most
interesting, perhaps, is iwyu_ast_util.h, which has routines  that make it
easier to navigate and analyze the clang AST.  There are also some STL helpers,
string helpers, filesystem helpers, etc.
  * iwyu_verrs.cc: debug logging for iwyu.
  * port.h: shim header for various non-portable constructs.
  * iwyu_getopt.cc: portability shim for GNU getopt(_long). Custom getopt(_long)
implementation for Windows.
  * fix_includes.py: the helper script that edits a file based on the iwyu
recommendations.


= Why Include What You Use? =

Are there any concrete benefits to a strict include-what-you-use policy? We like
to think so.


== Faster Compiles ==

Every .h file you bring in when compiling a source file lengthens the time to
compile, as the bytes have to be read, preprocessed, and parsed.  If you're not
actually using a .h file, you remove that cost.  With template code, where
entire instantiations have to be in .h files, this can be hundreds of thousands
of bytes of code.  In one case at Google, running include-what-you-use over a
.cc file improved its compile time by 30%.

Here, the main benefit of include-what-you-use comes from the flip side: "don't
include what you don't use."


== Fewer Recompiles ==

Many build tools, such as make, provide a mechanism for automatically figuring
out what .h files a .cc file depends on.  These mechanisms typically look at
#include lines.  When unnecessary #includes are listed, the build system is more
likely to recompile in cases where it's not necessary.

Again, the main advantage here is from "don't include what you don't use."


== Allow Refactoring ==

Suppose you refactor foo.h so it no longer uses vectors.  You'd like to remove
#include <vector> from foo.h, to reduce compile time -- template class files
such as vector can include a lot of code.  But can you?  In theory yes, but in
practice maybe not: some other file may be #including you and using vectors, and
depending (probably unknowingly) on your #include <vector> to compile.  Your
refactor could break code far away from you.

This is most compelling for a very large codebase (such as Google's).  In a
small codebase, it's practical to just compile everything after a refactor like
this, and clean up any errors you see.  When your codebase contains hundreds of
thousands of source files, identifying and cleaning up the errors can be a
project in itself.  In practice, people are likely to just leave the #include
<vector> line in there, even though it's unnecessary.

Here, it's the actual 'include what you use' policy that saves the day.  If
everyone who uses vector is #including <vector> themselves, then you can remove
<vector> without fear of breaking anything.


== Self-documentation ==

When you can trust the #include lines to accurately reflect what is used in the
file, you can use them to help you understand the code.  Looking at them, in
itself, can help you understand what this file needs in order to do its work.
If you use the optional 'commenting' feature of fix_includes.py, you can see
what symbols -- what functions and classes -- are used by this code.  It's like
a pared-down version of doxygen markup, but totally automated and present where
the code is (rather than in a separate web browser).

The 'commented' #include lines can also make it simpler to match function calls
and classes to the files that define them, without depending on a particular
IDE.

(The downside, of course, is the comments can get out of date as the code
changes, so unless you run iwyu often, you still have to take the comments with
a grain of salt.  Nothing is free. :-) )


== Dependency Cutting ==

Again, this makes the most sense for large code-bases.  Suppose your binaries
are larger than you would expect, and upon closer examination use symbols that
seem totally irrelevant.  Where do they come from?  Why are they there?  With
include-what-you-use, you can easily determine this by seeing who #includes the
files that define these symbols: those includers, and those alone, are
responsible for the use.

Once you know where a symbol is used in your binary, you can see how practical
it is to remove that use, perhaps by breaking up the relevant .h files into two
parts, and fixing up all callers.  Again it's iwyu to the rescue: with include-
what-you-use, figuring out the callers that need fixing is easy.


== Why Forward-Declare? ==

Include-what-you-use tries very hard to figure out when a forward-declare can be
used instead of an #include (iwyu would be about 90% less code if it didn't
bother with trying to forward-declare).

The reason for this is simple: if you can replace an #include by a forward-
declare, you reduce the code size, speeding up compiles as described above.  You
also make it easier to break dependencies: not only do you not depend on that
#include file, you no longer depend on everything it brings in.

There's a cost to forward-declaring as well: you lose the documentation features
mentioned above, that come with #include lines.  (A future version of iwyu may
mitigate this problem.)  And if a class changes -- for instance, it adds a new
default template argument -- you need to change many callsites, not just one.
It is also easier to accidentally violate the One Definition Rule when all you
expose is the name of a class (via a forward declare) rather than the full
definition (via an #include).

One compromise approach is to use 'forwarding headers', such as <iosfwd>.  These
forwarding headers could have comments saying where the definition of each
forward-declared class is.  Include-what-you-use does not currently support
forwarding headers, but may in the future.


= IWYU Mappings =

One of the difficult problems for IWYU is distinguishing between which header
contains a symbol definition and which header is the actual documented header to
include for that symbol.

For example, in GCC's libstdc++, std::unique_ptr<T> is defined in
<bits/unique_ptr.h>, but the documented way to get it is to #include <memory>.

Another example is NULL. Its authoritative header is <cstddef>, but for
practical purposes NULL is more of a keyword, and according to the standard it's
acceptable to assume it comes with <cstring>, <clocale>, <cwchar>, <ctime>,
<cstdio> or <cstdlib>. In fact, almost every standard library header pulls in
NULL one way or another, and we probably shouldn't force people to #include
<cstddef>.

To simplify IWYU deployment and command-line interface, many of these mappings
are compiled into the executable. These constitute the _default mappings_.

However, many mappings are toolchain- and version-dependent. Symbol homes and
#include dependencies change between releases of GCC and are dramatically
different for the standard libraries shipped with Microsoft Visual C++. Also,
mappings such as these are usually necessary for third-party libraries (e.g.
Boost, Qt) or even project-local symbols and headers as well.

Any mappings outside of the default set can therefore be specified as external
_mapping files_.


== Default Mappings ==

IWYU's default mappings are hard-coded in iwyu_include_picker.cc, and are very
GCC-centric. There are both symbol- and include mappings for GNU libstdc++ and
libc.


== Mapping Files ==

The mapping files conventionally use the .imp file extension, for "Iwyu
!MaPping" (terrible, I know). They use a JSON meta-format with the following
general form:

[
  { <directive>: <data> },
  { <directive>: <data> }
]

Directives can be one of the literal strings:
  * include
  * symbol
  * ref

and data varies between the directives, see below.

Note that you can mix directives of different kinds within the same mapping
file.

IWYU uses LLVM's YAML/JSON parser to interpret the mapping files, and it has
some idiosyncrasies:
  * Comments use a Python-style # prefix, not Javascript's //
  * Single-word strings can be left un-quoted

If the YAML parser is ever made more rigorous, it might be wise not to lean on
non-standard behavior, so apart from comment style, try to keep  mapping files
in line with the JSON spec.


=== Include Mappings ===

The include directive specifies a mapping between two include names (relative
path, including quotes or angle brackets.)

This is typically used to map from a private implementation detail header to a
public facade header, such as our <bits/unique_ptr.h> to <memory> example above.

Data for this directive is a list of four strings containing:
  * The include name to map from
  * The visibility of the include name to map from
  * The include name to map to
  * The visibility of the include name to map to

For example;

  { include: "private", "<memory>", "public" }

Most of the original mappings were generated with shell scripts (as evident from
the embedded comments) so there are several multi-step mappings from one private
header to another, to a third and finally to a public header. This reflects the
#include chain in the actual library headers. A hand-written mapping could be
reduced to one mapping per private header to its corresponding public header.

Include mappings support a special wildcard syntax for the first entry:

  { include: "private", "<public>", "public" }

The @ prefix is a signal that the remaining content is a regex, and can be used
to re-map a whole subdirectory of private headers to a public facade header.


=== Symbol Mappings ===

The symbol directive maps from a qualified symbol name to its authoritative
header.

Data for this directive is a list of four strings containing:
  * The symbol name to map from
  * The visibility of the symbol
  * The include name to map to
  * The visibility of the include name to map to

For example;

  { symbol: "private", "<cstddef>", "public" }

The symbol visibility is largely redundant -- it must always be private. It
isn't entirely clear why symbol visibility needs to be specified, and it might
be removed moving forward.

Like include, symbol directives support the @-prefixed regex syntax in the first
entry.


=== Mapping Refs ===

The last kind of directive, ref, is used to pull in another mapping file, much
like the C preprocessor's #include directive. Data for this directive is a
single string: the filename to include.

For example;

  { ref: "more.symbols.imp" },
  { ref: "/usr/lib/other.includes.imp" }

The rationale for the ref directive was to make it easier to compose project-
specific mappings from a set of library-oriented mapping files. For example,
IWYU might ship with mapping files for Boost, the SCL, various C standard
libraries, the Windows API, the Poco Library, etc. Depending on what your
specific project uses, you could easily create an aggregate mapping file with
refs to the relevant mappings.


=== Specifying Mapping Files ===

Mapping files are specified on the command-line using the --mapping_file switch:

  $ include-what-you-use -Xiwyu --mapping_file=foo.imp some_file.cc

The switch can be added multiple times to add more than one mapping file.

If the mapping filename is relative, it will be looked up relative to the
current directory.

ref directives are first looked up relative to the current directory and if not
found, relative to the referring mapping file.


= IWYU pragmas =

IWYU pragmas are used to give IWYU information that isn't obvious from the
source code, such as how different files relate to each other and which
#includes to never remove or include.

All pragmas start with "// IWYU pragma: " or "/* IWYU pragma: ". They are case-
sensitive and spaces are significant.


== IWYU pragma: keep ==

This pragma applies to a single #include statement. It forces IWYU to keep an
inclusion even if it is deemed unnecessary.

main.cc:
  #include <vector> // IWYU pragma: keep

In this case, std::vector isn't used, so <vector> would normally be discarded,
but the pragma instructs IWYU to leave it.


== IWYU pragma: export ==

This pragma applies to a single #include statement. It says that the current
file is to be considered the provider of any symbol from the included file.

facade.h:
  #include "detail/constants.h" // IWYU pragma: export
  #include "detail/types.h" // IWYU pragma: export
  #include <vector> // don't export stuff from <vector>

main.cc:
  #include "facade.h"

  // Assuming Thing comes from detail/types.h and MAX_THINGS from
detail/constants.h
  std::vector<Thing> things(MAX_THINGS);

Here, since detail/constants.h and detail/types.h have both been exported, IWYU
is happy with the facade.h include for Thing and MAX_THINGS.

In contrast, since <vector> has not been exported from facade.h, it will be
suggested as an additional include.


== IWYU pragma: begin_exports/end_exports ==

This pragma applies to a set of #include statements. It declares that the
including file is to be considered the provider of any symbol from these
included files. This is the same as decorating every #include statement with
IWYU pragma: export.

facade.h:
  // IWYU pragma: begin_exports
  #include "detail/constants.h"
  #include "detail/types.h"
  // IWYU pragma: end_exports

  #include <vector> // don't export stuff from <vector>


== IWYU pragma: private ==

This pragma applies to the current header file. It says that any symbol from
this file will be provided by another, optionally named, file.

private.h:
  // IWYU pragma: private, include "public.h"
  struct Private {};

private2.h:
  // IWYU pragma: private
  struct Private2 {};

public.h:
  #include "private.h"
  #include "private2.h"

main.cc:
  #include "private.h"
  #include "private2.h"

  Private p;
  Private2 i;

Using the type Private in main.cc will cause IWYU to suggest that you include
public.h.

Using the type Private2 in main.cc  will cause IWYU to suggest that you include
private2.h, but will also result in a warning that there's no public header for
private2.h.


== IWYU pragma: no_include ==

This pragma applies to the current source file. It declares that the named file
should not be suggested for inclusion by IWYU.

private.h:
  struct Private {};

unrelated.h:
  #include "private.h"
  ...

main.cc:
  #include "unrelated.h"
  // IWYU pragma: no_include "private.h"

  Private i;

The use of Private requires including private.h, but due to the no_include
pragma IWYU will not suggest private.h for inclusion. Note also that if you had
included private.h in main.cc, IWYU would suggest that the #include be removed.

This is useful when you know a symbol definition is already available via some
unrelated header, and you want to preserve that implicit dependency.

The no_include pragma is somewhat similar to private, but is employed at point
of use rather than at point of declaration.


== IWYU pragma: no_forward_declare ==

This pragma applies to the current source file. It says that the named symbol
should not be suggested for forward-declaration by IWYU.

public.h:
  struct Public {};

unrelated.h:
  struct Public;
  ...

main.cc:
  #include "unrelated.h" // declares Public
  // IWYU pragma: no_forward_declare Public

  Public* i;

IWYU would normally suggest forward-declaring Public directly in main.cc, but
no_forward_declare suppresses that suggestion. A forward-declaration for Public
is already available from unrelated.h.

This is useful when you know a symbol declaration is already available in a
source file via some unrelated header and you want to preserve that implicit
dependency, or when IWYU does not correctly understand that the definition is
necessary.


== IWYU pragma: friend ==

This pragma applies to the current header file. It says that any file matching
the given regular expression will be considered a friend, and is allowed to
include this header even if it's private. Conceptually similar to friend in C++.

If the expression contains spaces, it must be enclosed in quotes.

detail/private.h:
  // IWYU pragma: private
  // IWYU pragma: friend "detail/.*"
  struct Private {};

detail/alsoprivate.h:
  #include "detail/private.h"

  // IWYU pragma: private
  // IWYU pragma: friend "main\.cc"
  struct AlsoPrivate : Private {};

main.cc:
  #include "detail/alsoprivate.h"

  AlsoPrivate p;


== Which pragma should I use? ==

Ideally, IWYU should be smart enough to understand your intentions (and
intentions of the authors of libraries you use), so the first answer should
always be: none.

In practice, intentions are not so clear -- it might be ambiguous whether an
#include is there by clever design or by mistake, whether an #include serves to
export symbols from a private header through a public facade or if it's just a
left-over after some clean-up. Even when intent is obvious, IWYU can make
mistakes due to bugs or not-yet-implemented policies.

IWYU pragmas have some overlap, so it can sometimes be hard to choose one over
the other. Here's a guide based on how I understand them at the moment:

  * Use IWYU pragma: keep to force IWYU to keep any #include statement that
would be discarded under its normal policies.
  * Use IWYU pragma: export to tell IWYU that one header serves as the provider
for all symbols in another, included header (e.g. facade headers). Use IWYU
pragma: begin_exports/end_exports for a whole group of included headers.
  * Use IWYU pragma: no_include to tell IWYU that the file in which the pragma
is defined should never #include a specific header (the header may already be
included via some other #include.)
  * Use IWYU pragma: no_forward_declare to tell IWYU that the file in which the
pragma is defined should never forward-declare a specific symbol (a forward
declaration may already be available via some other #include.)
  * Use IWYU pragma: private to tell IWYU that the header in which the pragma is
defined is private, and should not be included directly.
  * Use IWYU pragma: private, include "public.h" to tell IWYU that the header in
which the pragma is defined is private, and public.h should always be included
instead.
  * Use IWYU pragma: friend ".*favorites.*" to override IWYU pragma: private
selectively, so that a set of files identified by a regex can include the file
even if it's private.

The pragmas come in three different classes;

  # Ones that apply to a single #include statement (keep, export)
  # Ones that apply to a file being included (private, friend)
  # Ones that apply to a file including other headers (no_include,
no_forward_declare)

Some files are both included and include others, so it can make sense to mix and
match.


= What Is a Use? =

(*Disclaimer:* the information here is accurate as of 12 May 2011, when it was
written.  Specifics of IWYU's policy, and even philosophy, may have changed
since then.  We'll try to remember to update this wiki page as that happens, but
may occasionally forget.  The further we are from May 2011, the more you should
take the below with a grain of salt.)

IWYU has the policy that you should #include a declaration for every symbol you
"use" in a file, or forward-declare it if possible.  But what does it mean to
"use" a symbol?

For the most part, IWYU considers a "use" the same as the compiler does: if you
get a compiler error saying "Unknown symbol 'foo'", then you are using foo.
Whether the use is a 'full' use, that needs the definition of the symbol, or a
'forward-declare' use, that can get by with just a declaration of the symbol,
likewise matches what the compiler allows.

This makes it sound like IWYU does the moral equivalent of taking a source file,
removing #include lines from it, seeing what the compiler complains about, and
marking uses as appropriate.  This is not what IWYU does.  Instead, IWYU does a
thought experiment: if the definition (or declaration) of a given type were not
available, would the code compile?  Here is an example illustrating the
difference:

foo.h:
  #include <ostream>
  typedef ostream OutputEmitter;

bar.cc:
  #include "foo.h"
  OutputEmitter oe;
  oe << 5;

Does bar.cc "use" ostream, such that it should #include <ostream>?  You'd hope
the answer would be no: the whole point of the OutputEmitter typedef,
presumably, is to hide the fact the type is an ostream.  Having to have clients
#include <ostream> rather defeats that purpose.  But iwyu sees that you're
calling operator<<(ostream, int), which is defined in <ostream>, so naively, it
should say that you need that header.

But IWYU doesn't (at least, modulo bugs).  This is because of its attempt to
analyze "author intent".


== Author Intent ==

If code has typedef Foo MyTypedef, and you write MyTypedef var;, you are using
MyTypedef, but are you also using Foo?  The answer depends on the _intent_ of
the person who wrote the typedef.

In the OutputEmitter example above, while we don't know for sure, we can guess
that the intent of the author was that clients should not be considered to use
the underlying type -- and thus they shouldn't have to #include <ostream>
themselves.  In that case, the typedef author takes responsibility for the
underlying type, promising to provide all the definitions needed to make code
compile.  The philosophy here is: "As long as you #include foo.h, you can use
OutputEmitter however you want, without worry of compilation errors."

Some typedef authors have a different intent.  <iosfwd> has the line

  typedef basic_ostream<char> ostream;

but it does *not* promise "as long as you #include <iosfwd>, you can use ostream
however you want, without worry of compilation errors."  For most uses of
ostream, you'll get a compiler error unles you #include <ostream> as well.

So take a slightly modified version of the above foo.h:

  #include <iosfwd>
  typedef ostream OutputEmitter;

This is a self-contained .h file: it's perfectly legal to typedef an incomplete
type (that's what iosfwd itself does).  But now iwyu had better tell bar.cc to
#include <ostream>, or it will break the build.  The difference is in the author
intent with the typedef.

Another case where author intent turns up is in function return types.  Consider
this function declaration:

  Foo* GetSingletonObject();   // Foo is defined in foo.h

If you write GetSingletonObject()->methodOnFoo(), are you "using"
Foo::methodOnFoo, such that you should #include foo.h?  Or are you supposed to
be able to operate on the results of GetSingletonObject without needing to
#include the definition of the returned type?  The answer is: it depends on the
author intent.  Sometimes the author is willing to provide the definition of the
return type, sometimes it is not.


=== Re-Exporting ===

When the author of a file is providing a definition of a symbol from somewhere
else, we say that the file is "re-exporting" that symbol.  In the first
OutputEmitter example, we say that foo.h is re-exporting ostream.  As a result,
people who #include foo.h get a definition of ostream along for free, even if
they don't directly #include <ostream> themselves.  Another way of thinking
about it is: if file A re-exports symbol B, we can pretend that A defines B,
even if it doesn't.

(In an ideal world, we'd have a very fine-grained concept: "File A re-exports
symbol S when it's used in the context of typedef T function F, or ...," but in
reality, we have the much looser concept "file A re-exports all symbols from
file B.")

A more accurate include-what-you-use rule is this: "If you use a symbol, you
must either #include the definition of the symbol, or #include a file that re-
exports the symbol."


== Manual re-export identifiers ==

You can mark that one file is re-exporting symbols from another via an IWYU
pragma in your source code:

  #include "private.h"   // IWYU pragma: export

This tells IWYU that if some other file uses symbols defined in private.h, they
can #include you to get them, if they want.

The full list of IWYU pragmas is defined at the top of iwyu_preprocessor.h.


== Automatic re-export ==

In certain situations, IWYU will decide that one file is exporting a symbol from
another even without the use of a pragma.  These are places where the author
intent is usually to re-export, such as with the typedef example above.  In each
of these cases, a simple technique can be used to override IWYU's decision to
re-export.


=== Automatic re-export: typedefs ===

If you write

  typedef Foo MyTypedef;

IWYU has to decide whether your file should re-export Foo or not.  Here is how
it gauges author intent:

  * If you (the typedef author), directly #include the definition of the
underlying type, then IWYU assumes you mean to re-export it.
  * If you (the typedef author), explicitly provide a forward-declare of the
underlying type, but do not directly #include its definition, then IWYU assumes
you do not mean to re-export it.
  * Otherwise, IWYU assumes you do not mean to re-export it.


  #include "foo.h"
  typedef Foo Typedef1;   // IWYU says you intend to re-export Foo

  class Bar;
  typedef Bar Typedef2;   // IWYU says you do not intend to re-export Bar

  #include "file_including_baz.h"   // does not define Baz itself
  typedef Baz Typedef3;   // IWYU says you do not intend to re-export Baz

If iwyu says you intend to re-export the underlying type, then nobody who uses
your typedef needs to #include the definition of the underlying type.  In
contrast, if iwyu says you do not intend to re-export the underlying type, then
everybody who uses your typedef needs to #include the definition of the
underlying type.

IWYU supports this in its analysis.  If you are using Typedef1 in your code and
#include "foo.h" anyway, iwyu will suggest you remove it, since you are getting
the definition of Foo via the typedef.


=== Automatic re-export: Function return values ===

The same rule applies with the return value in a function declaration:

  #include "foo.h"
  Foo Func1();   // IWYU says you intend to re-export Foo

  class Bar;
  Bar Func2();   // IWYU says you do not intend to re-export Bar

  #include "file_including_baz.h"
  Baz Func3();   // IWYU says you do not intend to re-export Baz

(Note that C++ is perfectly happy with a forward-declaration of the return type,
if the function is just being declared, and not defined.)

As of May 2011, the rule does *not* apply when returning a pointer or reference:

  #include "foo.h"
  Foo* Func1();   // IWYU says you do *not* intend to re-export Foo

  #include "bar.h"
  Bar& Func2();   // IWYU says you do *not* intend to re-export Bar

This is considered a bug, and the behavior will likely change in the future to
match the case where the functions return a class.

Here is an example of the rule in action:

foo.h:
  class Foo { ... }

bar.h:
  #include "foo.h"
  Foo CreateFoo() { ... }
  void ConsumeFoo(const Foo& foo) { ... }

baz.cc:
  #include "bar.h"
  ConsumeFoo(CreateFoo());

In this case, IWYU will say that baz.cc does not need to #include "foo.h", since
bar.h re-exports it.


=== Automatic re-export: Conversion constructors ===

Consider the following code:

foo.h:
  class Foo {
   public:
    Foo(int i) { ... };    // note: not an explicit constructor!
  };

bar.h:
  class Foo;
  void MyFunc(Foo foo);

baz.cc:
  #include "bar.h"
  MyFunc(11);

The above code does not compile, because the code to convert 11 to a Foo is not
visible to baz.cc.  Either baz.cc or bar.h needs to #include "foo.h" to make the
conversion constructor visible where MyFunc is being called.

The same rule applies as before:

  #include "foo.h"
  void Func1(Foo foo);   // IWYU says you intend to re-export Foo

  class Foo;
  void Func2(Foo foo);   // IWYU says you do not intend to re-export Foo

  #include "file_including_foo.h"
  void Func3(Foo foo);   // IWYU says you do not intend to re-export Foo

As before, if iwyu decides you do not intend to re-export Foo, then all callers
(in this case, baz.cc) need to.

The rule here applies even to const references (which can also be automatically
converted):

  #include "foo.h"
  void Func1(const Foo& foo);   // IWYU says you intend to re-export Foo



= Why Include What You Use Is Difficult =

This section is informational, for folks who are wondering why include-what-you-
use requires so much code and yet still has so many
errors.

Include-what-you-use has the most problems with templates and macros. If your
code doesn't use either, iwyu will probably do great. And, you're probably not
actually programming in C++...


== Use Versus Forward Declare ==

Include-what-you-use has to be able to tell when a symbol is being used in a way
that you can forward-declare it. Otherwise, if you wrote

  vector<MyClass*> foo;

iwyu would tell you to #include "myclass.h", when perhaps the whole reason
you're using a pointer here is to avoid the need for that #include.

In the above case, it's pretty easy for iwyu to tell that we can safely forward-
declare MyClass. But now consider

  vector<MyClass> foo;        // requires full definition of MyClass
  scoped_ptr<MyClass> foo;    // forward-declaring MyClass is often ok

To distinguish these, clang has to instantiate the vector and scoped_ptr
template classes, including analyzing all member variables
and the bodies of the constructor and destructor (and recursively for
superclasses).

But that's not enough: when instantiating the templates, we need to keep track
of which symbols come from template arguments and which
don't. For instance, suppose you call MyFunc<MyClass>(), where MyFunc looks like
this:

  template<typename T> void MyFunc() {
    T* t;
    MyClass myclass;
    ...
  }

In this case, the caller of MyFunc is not using the full type of MyClass,
because the template parameter is only used as a pointer. On
the other hand, the file that defines MyFunc is using the full type information
for MyClass. The end result is that the caller can forward-declare MyClass, but
the file defining MyFunc has to #include "myclass.h".


== Handling Template Arguments ==

Even figuring out what types are 'used' with a template can be difficult.
Consider the following two declarations:

  vector<MyClass> v;
  hash_set<MyClass> h;

These both have default template arguments, so are parsed like

  vector<MyClass, alloc<MyClass> > v;
  hash_set<MyClass, hash<MyClass>, equal_to<MyClass>, alloc<MyClass> > h;

What symbols should we say are used? If we say alloc<MyClass> is used when you
declare a vector, then every file that #includes <vector>
will also need to #include <memory>.

So it's tempting to just ignore default template arguments. But that's not right
either. What if hash<MyClass> is defined in some local myhash.h file (as
hash<string> often is)? Then we want to make sure iwyu says to #include
"myhash.h" when you create the hash_set (otherwise the code won't compile). That
requires paying attention to the default template argument. Figuring out how to
handle default template arguments can get very complex.

Even normal template arguments can be confusing. Consider this templated
function:

  template<typename A, typename B, typename C> void MyFunc(A (*fn)(B,C))
  { ... }

and you call MyFunc(FunctionReturningAFunctionPointer()). What types are being
used where, in this case?


== Who is Responsible for Dependent Template Types? ==

If you say vector<MyClass> v;, it's clear that you, and not vector.h are
responsible for the use of MyClass, even though all the functions that use
MyClass are defined in vector.h. (OK, technically, these functions are not
"defined" in a particular location, they're instantiated from template methods
written in vector.h, but for us it works out the same.)

When you say hash_map<MyClass, int> h;, you are likewise responsible for MyClass
(and int), but are you responsible for pair<MyClass, int>? That is the type that
hash_map uses to store your entries internally, and it depends on one of your
template arguments, but even so  it shouldn't be your responsibility -- it's an
implementation detail of hash_map. Of course, if you say hash_map<pair<int,
int>, int>, then you are responsible for the use of pair. Distinguishing these
two cases from each other, and from the vector case, can be difficult.

Now suppose there's a template function like this:

  template<typename T> void MyFunc(T t) {
    strcat(t, 'a');
    strchr(t, 'a');
    cerr << t;
  }

If you call MyFunc(some_char_star), which of these symbols are you responsible
for, and which is the author of MyFunc responsible for:
strcat, strchr, operator<<(ostream&, T)?

strcat is a normal function, and the author of MyFunc is responsible for its
use. This is an easy case.

In C++, strchr is a templatized function (different impls for char* and const
char*). Which version is called depends on the template argument. So, naively,
we'd conclude that the caller is responsible for the use of strchr. However,
that's ridiculous; we don't want caller of MyFunc to have to #include <string.h>
just to call MyFunc. We have special code that (usually) handles this kind of
case.

operator<< is also a templated function, but it's one that may be defined in
lots of different files. It would be ridiculous in its own way if MyFunc was
responsible for #including every file that defines operator<<(ostream&, T) for
all T. So, unlike the two cases above, the caller is the one responsible for the
use of  operator<<, and will have to #include the file that defines it. It's
counter-intuitive, perhaps, but the alternatives are all worse.

As you can imagine, distinguishing all these cases is extremely difficult. To
get it exactly right would require re-implementing C++'s (byzantine) lookup
rules, which we have not yet tackled.


== Template Template Types ==

Let's say you have a function

  template<template<typename U> T> void MyFunc() {
    T<string> t;
  }

And you call MyFunc<hash_set>. Who is responsible for the 'use' of hash<string>,
and thus needs to #include "myhash.h"? I think it has to be the caller, even if
the  caller never uses the string type in its file at all. This is rather
counter-intuitive. Luckily, it's also rather rare.


== Typedefs ==

Suppose you #include a file "foo.h" that has typedef hash_map<Foo, Bar> MyMap;.
And you have this code:

  for (MyMap::iterator it = ...)

Who, if anyone, is using the symbol hash_map<Foo, Bar>::iterator? If we say you,
as the author of the for-loop, are the user, then you must #include <hash_map>,
which undoubtedly goes against the goal of the typedef (you shouldn't even have
to know you're using a hash_map). So we want to say the author of the typedef is
responsible for the use. But how could the author of the typedef know that you
were going to use MyMap::iterator? It can't predict that. That means it has to
be responsible for every possible use of the typedef type. This can be
complicated to figure out. It requires instantiating all methods of the
underlying type, some of which might not even be legal C++ (if, say, the class
uses SFINAE).

Worse, when the language auto-derives template types, it loses typedef
information. Suppose you wrote this:

  MyMap m;
  find(m.begin(), m.end(), some_foo);

The compiler sees this as syntactic sugar for find<hash_map<Foo, Bar, hash<Foo>,
equal_to<Foo>, alloc<Foo> >(m.begin(), m.end(), some_foo);

Not only is the template argument hash_map instead of MyMap, it includes all the
default template arguments, with no indication they're default arguments. All
the tricks we used above to intelligently ignore default template arguments are
worthless here. We have to jump through lots of hoops so this code doesn't
require you to #include not only <hash_map>, but <alloc> and <utility> as well.


== Macros ==

It's no surprise macros cause a huge problem for include-what-you-use.
Basically, all the problems of templates also apply to macros, but worse: with
templates you can analyze the uninstantiated template, but with macros, you
can't analyze the uninstantiated macro -- it likely doesn't even parse cleanly
in isolation. As a result, we have very few tools to distinguish when the author
of a macro is responsible for a symbol used in a macro, and when the caller of
the macro is responsible.


== Includes with Side Effects ==

While not a major problem, this indicates the myriad "gotchas" that exist around
include-what-you-use: removing an #include and replacing it with a forward-
declare may be dangerous even if no symbols are fully used from the #include.
Consider the following code:

foo.h:
  namespace ns { class Foo {}; }
  using ns::Foo;

foo.cc:
  #include "foo.h"
  Foo* foo;

If iwyu just blindly replaces the #include with a forward declare such as
namespace ns { class Foo; }, the code will break because of the lost using
declaration. Include-what-you-use has to watch out for this case.

Another case is a header file like this:

foo.h:
  #define MODULE_NAME MyModule
  #include "module_writer.h"

We might think we can remove an #include of foo.h and replace it by #include
module_writer.h, but that is likely to break the build if module_writer.h
requires MODULE_NAME be defined.  Since my file doesn't participate in this
dependency at all, it won't even notice it.  IWYU needs to keep track of
dependencies between files it's not even trying to analyze!


== Private Includes ==

Suppose you write vector<int> v;. You are using vector, and thus have to
#include <vector>. Even this seemingly easy case is difficult, because vector
isn't actually defined in <vector>; it's defined in <bits/stl_vector.h>. The C++
standard library has hundreds of private files that users are not supposed to
#include directly. Third party libraries have hundreds more.  There's no general
way to distinguish private from public headers; we have to manually construct
the proper mapping.

In the future, we hope to provide a way for users to annotate if a file is
public or private, either a comment or a #pragma. For now, we hard-code it in
the iwyu tool.

The mappings themselves can be ambiguous. For instance, NULL is provided by many
files, including stddef.h, stdlib.h, and more. If you use NULL, what #include
file should iwyu suggest? We have rules to try to minimize the number of
#includes you have to add; it can get rather involved.


== Unparsed Code ==

Conditional #includes are a problem for iwyu when the condition is false:

  #if _MSC_VER
  #include <foo>
  #endif

If we're not running under windows (and iwyu does not currently run under
windows), we have no way of telling if foo is a necessary #include or not.


== Placing New Includes and Forward-Declares ==

Figuring out where to insert new #includes and forward-declares is a complex
problem of its own (one that is the responsibility of fix_includes.py). In
general, we want to put new #includes with existing #includes. But the existing
#includes may be broken up into sections, either because of conditional
#includes (with #ifdefs), or macros (such as #define __GNU_SOURCE), or for other
reasons. Some forward-declares may need to come early in the file, and some may
prefer to come later (after we're in an appropriate namespace, for instance).

fix_includes.py tries its best to give pleasant-looking output, while being
conservative about putting code in a place where it might not compile. It uses
heuristics to do this, which are not yet perfect.
