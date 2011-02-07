====================
Include What You Use
====================

"Include what you use" means this: for every symbol (type, function,
variable, or macro) that you use in foo.cc (or foo.cpp), either foo.cc
or foo.h should #include a .h file that exports the declaration of
that symbol. (Similarly, for foo_test.cc, either foo_test.cc or foo.h
should do the #including.)  Obviously symbols defined in foo.cc itself
are excluded from this requirement.

This puts us in a state where every file includes the headers it needs
to declare the symbols that it uses.  When every file includes what it
uses, then it is possible to edit any file and remove unused headers,
without fear of accidentally breaking the upwards dependencies of that
file.  It also becomes easy to automatically track and update
dependencies in the source code.

======
CAVEAT
======

This is alpha quality software -- at best.  It was written to work
specifically in the Google source tree, and may make assumptions, or
have gaps, that are immediately and embarrassingly evident in other
types of code.  For instance, we only run this on C++ code, not C or
Objective C.  Even for Google code, the tool still makes a lot of
mistakes.

While we work to get IWYU quality up, we will be stinting new
features, and will prioritize reported bugs along with the many
existing, known bugs.  The best chance of getting a problem fixed is
to submit a patch that fixes it (along with a unittest case that
verifies the fix)!

============
How to Build
============

You will need the clang and llvm trees on your system, such as by
checking out their SVN trees:
   http://clang.llvm.org/get_started.html

Then download the include-what-you-use tarball and unpack it the
/path/to/llvm/tools/clang/tools directory:
   llvm/tools/clang/tools$ tar xfz include-what-you-use-<whatever>.tar.gz
Or, alternately, get the project directly from svn:
   llvm/tools/clang/tools$ svn co http://include-what-you-use.googlecode.com/svn/trunk/ include-what-you-use

Then cd into the include-what-you-use directory (under tools) and type
'make'.

Include what you use makes heavy use of clang internals, and will
occasionally break when clang is updated.  For best results, download
clang as of the same revision number of the last include-what-you-use
release.  You can find this revision number in comments at the top
of the include-what-you-use Makefile.

==========
How to Run
==========

The easiest way to run IWYU over your codebase is to run
   make -k CXX=/path/to/llvm/Debug+Asserts/bin/include-what-you-use
or
   make -k CXX=/path/to/llvm/Release/bin/include-what-you-use

(include-what-you-use always exits with an error code, so the build
system knows it didn't build a .o file.  Hence the need for -k.)

We also include, in this directory, a tool that automatically fixes up
your source files based on the iwyu recommendations.  This is also
alpha-quality software!  Here's how to use it (requires python):

   make -k CXX=/path/to/llvm/Debug+Asserts/bin/include-what-you-use > /tmp/iwyu.out
   python fix_includes.py < /tmp/iwyu.out

If you don't like the way fix_includes.py munges your #include lines,
you can control its behavior via flags. fix_includes.py --help will
give a full list, but these are some common ones:
    * -b: Put blank lines between system and Google #includes
    * --nocomments: Don't add the 'why' comments next to #includes

WARNING: include-what-you-use only analyzes .cc (or .cpp) files built
by 'make', along with their corresponding .h files.  If your project
has a .h file with no corresponding .cc file, iwyu will ignore it.
include-what-you-use supports the AddGlobToReportIWYUViolationsFor()
function which can be used to indicate other files to analyze, but
it's not currently exposed to the user in any way.

============================
How to Correct IWYU Mistakes
============================

1) If fix_includes.py has removed an #include you actually need, add
   it back in with the comment // NOLINT(iwyu) at the end of the
   #include line.  Note that the comment is case-sensitive.

2) If fix_includes has added an #include you don't need, just take it
   out.  We hope to come up with a more permanent way of fixing later.

3) If fix_includes has wrongly added or removed a forward-declare,
   just fix it up manually.

4) If fix_includes has suggested a private header file (such as
   <bits/stl_vector.h>) instead of the proper public header file
   (<vector>), you can fix this by inserting a specially crafted
   comment near the top of the public file (assuming you can write to
   it).  These so-called 'iwyu pragma' comments are still being
   designed, and the actual syntax is in flux.  We should be keeping
   documentation at the top of iwyu_preprocessor.h, but beware the
   syntax may change over time.

=================
Running the Tests
=================

To run the iwyu tests, run
   python run_iwyu_tests.py

It runs one test for each .cc file in the tests/ directory.  (We have
additional tests in more_tests/, but have not yet gotten the testing
framework set up for those tests.)  The output can be a bit hard to
read, but if a test fails, the reason why will be listed after the
'ERROR:root:Test failed for xxx' line.

If fixing a bug in clang, please add a test to the test suite!  You
can create a file called whatever.cc (*not* .cpp), and whatever.h, and
whatever-<extension>.h.  You may be able to get away without adding
any .h files, and just #including "direct.h" -- see, for instance,
tests/remove_fwd_decl_when_including.cc.

When fixing fix_includes.py, add a test case to fix_includes_test.py
and run
   python fix_includes_test.py

=========
Debugging
=========

It's possible to run include-what-you-use in gdb, to debug that way.
Another useful tool -- especially in combination with gdb -- is to get
the verbose include-what-you-use output.  See iwyu_output.h for a
description of the verbose levels.  Level 7 is very verbose -- it
dumps basically the entire AST as it's being traversed, along with
iwyu decisions made as it goes -- but very useful for that:
   env IWYU_VERBOSE=7 make -k CXX=/path/to/llvm/Debug+Asserts/bin/include-what-you-use 2>&1 > /tmp/iwyu.verbose

===============
Developer Notes
===============

The codebase is strewn with TODOs of known problems, and also language
constructs that aren't adequately tested yet.  So there's plenty to
do!  Here's a brief guide through the codebase:

* iwyu.cpp: the main file, it includes the logic for deciding when a
  symbol has been 'used', and whether it's a full use (definition
  required) or forward-declare use (only a declaration required).  It
  also inclues the logic for following uses through template
  instantiations.

* iwyu_output.cpp: the file that translates from 'uses' into iwyu
  violations.  This has the logic for deciding if a use is covered by
  an existing #include (or is a built-in).  It also, as the name
  suggests, prints the iwyu output.

* iwyu_preprocessor.cpp: handles the preprocessor directives, the
  #includes and #ifdefs, to construct the existing include-tree.  This
  is obviously essential for include-what-you-use analysis.  This file
  also handles the iwyu pragma-comments.

* iwyu_include_picker.cpp: this finds canonical #includes, handling
  hard-coded private->public mappings (like bits/stl_vector.h ->
  vector) and symbols with multiple possible #includes (like NULL).

* iwyu_cache.cpp: holds the cache of instantiated templates (may hold
  other cached info later).  This is data that is expensive to compute
  and may be used more than once.

* iwyu_globals.cpp: holds various global variables.  We used to think
  globals were bad, until we saw how much having this file simplified
  the code...

* iwyu_*_util(s).h and .cpp: utility functions of various types.  The
  most interesting, perhaps, is iwyu_ast_util.h, which has routines
  that make it easier to navigate and analyze the clang AST.  There
  are also some STL helpers, string helpers, filesystem helpers, etc.

* fix_includes.py: the helper script that edits a file based on the
  iwyu recommendations.

=====================
Why IWYU is Difficult
=====================

This last section is informational, for folks who are wondering why
include-what-you-use requires so much code and yet still has so many
errors.

Include-what-you-use has the most problems with templates and
macros. If your code doesn't use either, iwyu will probably do
great. And, you're probably not actually programming in C++...

USE VERSUS FORWARD DECLARE

Include-what-you-use has to be able to tell when a symbol is being
used in a way that you can forward-declare it. Otherwise, if you wrote

   vector<MyClass*> foo;

iwyu would tell you to #include "myclass.h", when perhaps the whole
reason you're using a pointer here is to avoid the need for that
#include.

In the above case, it's pretty easy for iwyu to tell that we can
safely forward-declare MyClass. But now consider

   vector<MyClass> foo;        // requires full definition of MyClass
   scoped_ptr<MyClass> foo;    // forward-declaring MyClass is ok

To distinguish these, clang has to instantiate the vector and
scoped_ptr template classes, including analyzing all member variables
and the bodies of the constructor and destructor (and recursively for
superclasses).

But that's not enough: when instantiating the templates, we need to
keep track of which symbols come from template arguments and which
don't. For instance, suppose you call MyFunc<MyClass>(), where MyFunc
looks like this:

   template<typename T> void MyFunc() {
      T* t;
      MyClass myclass;
      ...
   }

In this case, the caller of MyFunc is not using the full type of
MyClass, because the template parameter is only used as a pointer. On
the other hand, the file that defines MyFunc is using the full type
information for MyClass. The end result is that the caller can
forward-declare MyClass, but the file defining MyFunc has to #include
"myclass.h".

HANDLING TEMPLATE ARGUMENTS

Even figuring out what types are 'used' with a template can be
difficult. Consider the following two declarations:

   vector<MyClass> v;
   hash_set<MyClass> h;

These both have default template arguments, so are parsed like

   vector<MyClass, alloc<MyClass> > v;
   hash_set<MyClass, hash<MyClass>, equal_to<MyClass>, alloc<MyClass> > h;

What symbols should we say are used? If we say alloc<MyClass> is used
when you declare a vector, then every file that #includes <vector>
will also need to #include <memory>.

So it's tempting to just ignore default template arguments. But that's
not right either. What if hash<MyClass> is defined in some local
myhash.h file (as hash<string> often is)? Then we want to make sure iwyu
says to #include "myhash.h" when you create the hash_set
(otherwise the code won't compile). That requires paying attention to
the default template argument. Figuring out how to handle default
template arguments can get very complex.

Even normal template arguments can be confusing. Consider this
templated function:

template<typename A, typename B, typename C> void MyFunc(A (*fn)(B,C)) { ... }

and you call MyFunc(FunctionReturningAFunctionPointer()). What types
are being used where, in this case?

WHO IS RESPONSIBLE FOR DEPENDENT TEMPLATE TYPES?

If you say vector<MyClass> v;, it's clear that you, and not vector.h,
are responsible for the use of MyClass, even though all the functions
that use MyClass are defined in vector.h. (OK, technically, these
functions are not "defined" in a particular location, they're
instantiated from template methods written in vector.h, but for us it
works out the same.)

When you say hash_map<MyClass, int> h;, you are likewise responsible
for MyClass (and int), but are you responsible for pair<MyClass, int>?
That is the type that hash_map uses to store your entries internally,
and it depends on one of your template arguments, but even so it
shouldn't be your responsibility -- it's an implementation detail of
hash_map. Of course, if you say hash_map<pair<int, int>, int>, then
you are responsible for the use of pair. Distinguishing these two
cases from each other, and from the vector case, can be difficult.

Now suppose there's a template function like this:

   template<typename T> void MyFunc(T t) {
      strcat(t, 'a');
      strchr(t, 'a');
      cerr << t;
   }

If you call MyFunc(some_char_star), which of these symbols are you
responsible for, and which is the author of MyFunc responsible for:
strcat, strchr, operator<<(ostream&, T)?

strcat is a normal function, and the author of MyFunc is responsible
for its use. This is an easy case.

In C++, strchr is a templatized function (different impls for char*
and const char*). Which version is called depends on the template
argument. So, naively, we'd conclude that the caller is responsible
for the use of strchr. However, that's ridiculous; we don't want every
caller of MyFunc to have to #include <string.h> just to call
MyFunc. We have special code that (usually) handles this kind of case.

operator<< is also a templatized function, but it's one that may be
defined in lots of different files. It would be ridiculous in its own
way if MyFunc was responsible for #including every file that defines
operator<<(ostream&, T) for all T. So, unlike the two cases above, the
caller is the one responsible for the use of operator<<, and will have
to #include the file that defines it. It's counter-intuitive, perhaps,
but the alternatives are all worse.

As you can imagine, distinguishing all these cases is extremely
difficult. To get it exactly right would require re-implementing C++'s
(byzantine) lookup rules, which we have not yet tackled.

TEMPLATE TEMPLATE TYPES

Let's say you have a function

   template<template<typename U> T> void MyFunc() {
     T<string> t;
   }

and you call MyFunc<hash_set>(). Who is responsible for the 'use' of
hash<string>, and thus needs to #include "myhash.h"? I think
it has to be the caller, even if the caller never uses the string type
in its file at all. This is rather counter-intuitive. Luckily, it's
also rather rare.

TYPEDEFS

Suppose you #include a file "foo.h" that has typedef hash_map<Foo,
Bar> MyMap;. And you have this code:

   for (MyMap::iterator it = ...)

Who, if anyone, is using the symbol hash_map<Foo, Bar>::iterator? If
we say you, as the author of the for-loop, are the user, then you must
#include <hash_map>, which undoubtedly goes against the goal of the
typedef (you shouldn't even have to know you're using a hash_map). So
we want to say the author of the typedef is responsible for the
use. But how could the author of the typedef know that you were going
to use MyMap::iterator? It can't predict that. That means it has to be
responsible for every possible use of the typedef type. This can be
complicated to figure out. It requires instantiating all methods of
the underlying type, some of which might not even be legal C++ (if,
say, the class uses SFINAE).

Worse, when the language auto-derives template types, it loses typedef
information. Suppose you wrote this:

   MyMap m;
   find(m.begin(), m.end(), some_foo);

The compiler sees this as syntactic sugar for find<hash_map<Foo, Bar,
hash<Foo>, equal_to<Foo>, alloc<Foo> >(m.begin(), m.end(), some_foo);

Not only is the template argument hash_map instead of MyMap, it
includes all the default template arguments, with no indication
they're default arguments. All the tricks we used above to
intelligently ignore default template arguments are worthless here. We
have to jump through lots of hoops so this code doesn't require you to
#include not only <hash_map>, but <alloc> and <utility> as well.

MACROS

It's no surprise macros cause a huge problem for include-what-you-use.
Basically, all the problems of templates also apply to macros, but
worse: we can analyze an uninstantiated template, but we can't analyze
an uninstantiated macro -- the macro likely doesn't even parse cleanly
in isolation. As a result, we have very few tools to distinguish when
the author of a macro is responsible for a symbol used in a macro, and
when the caller of the macro is responsible.

PRIVATE INCLUDES

Suppose you write vector<int> v;. You are using vector, and thus have
to #include <vector>. Even this seemingly easy case is difficult,
because vector isn't actually defined in <vector>; it's defined in
<bits/stl_vector.h>. The C++ standard library has hundreds of private
files that users are not supposed to #include directly. Third party
libraries have hundreds more. There's no general way to distinguish
private from public headers; we have to manually construct the proper
mapping.

In the future, we hope to provide a way for users to annotate if a
file is public or private, either a comment or a #pragma. For now, we
hard-code it in the iwyu tool.

The mappings themselves can be ambiguous. For instance, NULL is
provided by many files, including stddef.h, stdlib.h, and more. If you
use NULL, what #include file should iwyu suggest? We have rules to try
to minimize the number of #includes you have to add; it can get rather
involved.

UNPARSED CODE

Conditional #includes are a problem for iwyu when the condition is
false:

#if _MSC_VER
#include <foo>
#endif

If we're not running under windows (and iwyu does not currently run
under windows), we have no way of telling if foo is a necessary
#include or not.

PLACING NEW INCLUDES AND FORWARD-DECLARES

Figuring out where to insert new #includes and forward-declares is a
complex problem of its own (one that is the responsibility of
fix_includes.py). In general, we want to put new #includes with
existing #includes. But the existing #includes may be broken up into
sections, either because of conditional #includes (with #ifdefs), or
macros (such as #define __GNU_SOURCE), or for other reasons. Some
forward-declares may need to come early in the file, and some may
prefer to come later (after we're in an appropriate namespace, for
instance).

fix_includes.py tries its best to give pleasant-looking output, while
being conservative about putting code in a place where it might not
compile. It uses heuristics to do this, which are not yet perfect.
