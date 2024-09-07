# Why include-what-you-use is difficult #

This section is informational, for folks who are wondering why
include-what-you-use requires so much code and yet still has so many errors.

Include-what-you-use has the most problems with templates and macros. If your
code doesn't use either, IWYU will probably do great. And, you're probably not
actually programming in C++...


## Use versus forward declare ##

Include-what-you-use has to be able to tell when a symbol is being used in a way
that you can forward-declare it. Otherwise, if you wrote

    vector<MyClass*> foo;

IWYU would tell you to `#include "myclass.h"`, when perhaps the whole reason
you're using a pointer here is to avoid the need for that `#include`.

In the above case, it's pretty easy for IWYU to tell that we can safely
forward-declare `MyClass`. But now consider

    vector<MyClass> foo;        // requires full definition of MyClass
    scoped_ptr<MyClass> foo;    // forward-declaring MyClass is often ok

To distinguish these, clang has to instantiate the vector and scoped_ptr
template classes, including analyzing all member variables and the bodies of the
constructor and destructor (and recursively for superclasses).

But that's not enough: when instantiating the templates, we need to keep track
of which symbols come from template arguments and which don't. For instance,
suppose you call `MyFunc<MyClass>()`, where `MyFunc` looks like this:

    template<typename T> void MyFunc() {
      T* t;
      MyClass myclass;
      ...
    }

In this case, the caller of `MyFunc` is not using the full type of `MyClass`,
because the template parameter is only used as a pointer. On the other hand, the
file that defines `MyFunc` is using the full type information for `MyClass`. The
end result is that the caller can forward-declare `MyClass`, but the file
defining `MyFunc` has to `#include "myclass.h"`.


## Handling template arguments ##

Even figuring out what types are 'used' with a template can be
difficult. Consider the following two declarations:

    vector<MyClass> v;
    hash_set<MyClass> h;

These both have default template arguments, so are parsed like

    vector<MyClass, alloc<MyClass> > v;
    hash_set<MyClass, hash<MyClass>, equal_to<MyClass>, alloc<MyClass> > h;

What symbols should we say are used? If we say `alloc<MyClass>` is used when you
declare a vector, then every file that `#includes` `<vector>` will also need to
`#include <memory>`.

So it's tempting to just ignore default template arguments. But that's not right
either. What if `hash<MyClass>` is defined in some local `myhash.h` file (as
`hash<string>` often is)? Then we want to make sure IWYU says to `#include
"myhash.h"` when you create the hash_set (otherwise the code won't
compile). That requires paying attention to the default template
argument. Figuring out how to handle default template arguments can get very
complex.

Even normal template arguments can be confusing. Consider this templated
function:

    template<typename A, typename B, typename C> void MyFunc(A (*fn)(B,C))
    { ... }

and you call `MyFunc(FunctionReturningAFunctionPointer())`. What types are being
used where, in this case?


## Who is responsible for dependent template types? ##

If you say `vector<MyClass> v;`, it's clear that you, and not `vector.h` are
responsible for the use of `MyClass`, even though all the functions that use
`MyClass` are defined in `vector.h`. (OK, technically, these functions are not
"defined" in a particular location, they're instantiated from template methods
written in `vector.h`, but for us it works out the same.)

When you say `hash_map<MyClass, int> h;`, you are likewise responsible for
`MyClass` (and `int`), but are you responsible for `pair<MyClass, int>`? That is
the type that hash_map uses to store your entries internally, and it depends on
one of your template arguments, but even so it shouldn't be your responsibility
-- it's an implementation detail of hash_map. Of course, if you say
`hash_map<pair<int, int>, int>`, then you are responsible for the use of
`pair`. Distinguishing these two cases from each other, and from the vector
case, can be difficult.

Now suppose there's a template function like this:

    template<typename T> void MyFunc(T t) {
      strcat(t, 'a');
      strchr(t, 'a');
      cerr << t;
    }

If you call `MyFunc(some_char_star)`, which of these symbols are you responsible
for, and which is the author of `MyFunc` responsible for: `strcat`, `strchr`,
`operator<<(ostream&, T)`?

`strcat` is a normal function, and the author of `MyFunc` is responsible for its
use. This is an easy case.

In C++, `strchr` is a templatized function (different impls for `char*` and
`const char*`). Which version is called depends on the template argument. So,
naively, we'd conclude that the caller is responsible for the use of
`strchr`. However, that's ridiculous; we don't want caller of `MyFunc` to have
to `#include <string.h>` just to call `MyFunc`. We have special code that
(usually) handles this kind of case.

`operator<<` is also a templated function, but it's one that may be defined in
lots of different files. It would be ridiculous in its own way if `MyFunc` was
responsible for including every file that defines `operator<<(ostream&, T)` for
all `T`. So, unlike the two cases above, the caller is the one responsible for
the use of `operator<<`, and will have to `#include` the file that defines
it. It's counter-intuitive, perhaps, but the alternatives are all worse.

As you can imagine, distinguishing all these cases is extremely difficult. To
get it exactly right would require re-implementing C++'s (byzantine) lookup
rules, which we have not yet tackled.


## Template template types ##

Let's say you have a function

    template<template<typename U> T> void MyFunc() {
      T<string> t;
    }

And you call `MyFunc<hash_set>`. Who is responsible for the 'use' of
`hash<string>`, and thus needs to `#include "myhash.h"`? I think it has to be
the caller, even if the caller never uses the `string` type in its file at
all. This is rather counter-intuitive. Luckily, it's also rather rare.


## Typedefs ##

Suppose you `#include` a file `"foo.h"` that has typedef `hash_map<Foo, Bar>
MyMap;`. And you have this code:

    for (MyMap::iterator it = ...)

Who, if anyone, is using the symbol `hash_map<Foo, Bar>::iterator`? If we say
you, as the author of the for-loop, are the user, then you must `#include
<hash_map>`, which undoubtedly goes against the goal of the typedef (you
shouldn't even have to know you're using a hash_map). So we want to say the
author of the typedef is responsible for the use. But how could the author of
the typedef know that you were going to use `MyMap::iterator`? It can't predict
that. That means it has to be responsible for every possible use of the typedef
type. This can be complicated to figure out. It requires instantiating all
methods of the underlying type, some of which might not even be legal C++ (if,
say, the class uses SFINAE).

Worse, when the language auto-derives template types, it loses typedef
information. Suppose you wrote this:

    MyMap m;
    find(m.begin(), m.end(), some_foo);

The compiler sees this as syntactic sugar for `find<hash_map<Foo, Bar,
hash<Foo>, equal_to<Foo>, alloc<Foo> >(m.begin(), m.end(), some_foo);`

Not only is the template argument `hash_map` instead of `MyMap`, it includes all
the default template arguments, with no indication they're default
arguments. All the tricks we used above to intelligently ignore default template
arguments are worthless here. We have to jump through lots of hoops so this code
doesn't require you to `#include` not only `<hash_map>`, but `<alloc>` and
`<utility>` as well.


## Macros ##

It's no surprise macros cause a huge problem for
include-what-you-use. Basically, all the problems of templates also apply to
macros, but worse: with templates you can analyze the uninstantiated template,
but with macros, you can't analyze the uninstantiated macro -- it likely doesn't
even parse cleanly in isolation. As a result, we have very few tools to
distinguish when the author of a macro is responsible for a symbol used in a
macro, and when the caller of the macro is responsible.


## Includes with side effects ##

While not a major problem, this indicates the myriad "gotchas" that exist around
include-what-you-use: removing an `#include` and replacing it with a
forward-declare may be dangerous even if no symbols are fully used from the
`#include`.  Consider the following code:

    foo.h:
      namespace ns { class Foo {}; }
      using ns::Foo;

    foo.cc:
      #include "foo.h"
      Foo* foo;`

If IWYU just blindly replaces the `#include` with a forward declare such as
`namespace ns { class Foo; }`, the code will break because of the lost using
declaration. Include-what-you-use has to watch out for this case.

Another case is a header file like this:

    foo.h:
      #define MODULE_NAME MyModule
      #include "module_writer.h"

We might think we can remove an `#include` of `foo.h` and replace it by
`#include "module_writer.h"`, but that is likely to break the build if
`module_writer.h` requires `MODULE_NAME` be defined.  Since my file doesn't
participate in this dependency at all, it won't even notice it.  IWYU needs to
keep track of dependencies between files it's not even trying to analyze!


## Private includes ##

Suppose you write `vector<int> v;`. You are using vector, and thus have to
`#include <vector>`. Even this seemingly easy case is difficult, because vector
isn't actually defined in `<vector>`; it's defined in `<bits/stl_vector.h>`. The
C++ standard library has hundreds of private files that users are not supposed
to `#include` directly. Third party libraries have hundreds more.  There's no
general way to distinguish private from public headers; we have to manually
construct the proper mapping.

In the future, we hope to provide a way for users to annotate if a file is
public or private, either a comment or a `#pragma`. For now, we hard-code it in
the IWYU tool.

The mappings themselves can be ambiguous. For instance, `NULL` is provided by
many files, including `stddef.h`, `stdlib.h`, and more. If you use `NULL`, what
header file should IWYU suggest? We have rules to try to minimize the number of
`#includes` you have to add; it can get rather involved.


## Unparsed code ##

Conditional `#includes` are a problem for IWYU when the condition is false:

    #if defined(LOG_VERBOSE)
    #include <verbose_logger.h>
    #endif

    ...

    void StartProcess() {
        #if defined(LOG_VERBOSE)
        LogVerbose("Starting process");
        #endif
        ...
    }

If you're running IWYU without that preprocessor definition set, it has no way
of telling if `verbose_logger.h` is a necessary `#include` or not.


## Placing new includes and forward-declares ##

Figuring out where to insert new `#includes` and forward-declares is a complex
problem of its own (one that is the responsibility of `fix_includes.py`). In
general, we want to put new `#includes` with existing `#includes`. But the
existing `#includes` may be broken up into sections, either because of
conditional `#includes` (with `#ifdefs`), or macros (such as `#define
__GNU_SOURCE`), or for other reasons. Some forward-declares may need to come
early in the file, and some may prefer to come later (after we're in an
appropriate namespace, for instance).

`fix_includes.py` tries its best to give pleasant-looking output, while being
conservative about putting code in a place where it might not compile. It uses
heuristics to do this, which are not yet perfect.
