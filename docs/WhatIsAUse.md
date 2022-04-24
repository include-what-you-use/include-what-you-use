# What is a use? #

(*Disclaimer:* the information here is accurate as of 12 May 2011, when it was written.  Specifics of IWYU's policy, and even philosophy, may have changed since then.  We'll try to remember to update this file as that happens, but may occasionally forget.  The further we are from May 2011, the more you should take the below with a grain of salt.)

IWYU has the policy that you should `#include` a declaration for every symbol you "use" in a file, or forward-declare it if possible.  But what does it mean to "use" a symbol?

For the most part, IWYU considers a "use" the same as the compiler does: if you get a compiler error saying "Unknown symbol 'foo'", then you are using `foo`.  Whether the use is a 'full' use, that needs the definition of the symbol, or a 'forward-declare' use, that can get by with just a declaration of the symbol, likewise matches what the compiler allows.

This makes it sound like IWYU does the moral equivalent of taking a source file, removing `#include` lines from it, seeing what the compiler complains about, and marking uses as appropriate.  This is not what IWYU does.  Instead, IWYU does a thought experiment: if the definition (or declaration) of a given type were not available, would the code compile?  Here is an example illustrating the difference:

    foo.h:
      #include <ostream>
      typedef std::ostream OutputEmitter;

    bar.cc:
      #include "foo.h"
      OutputEmitter oe;
      oe << 5;

Does `bar.cc` "use" `std::ostream`, such that it should `#include <ostream>`?  You'd hope the answer would be no: the whole point of the `OutputEmitter` typedef, presumably, is to hide the fact the type is an `std::ostream`.  Having to have clients `#include <ostream>` rather defeats that purpose.  But IWYU sees that you're calling `operator<<(std::ostream&, int)`, which is defined in `<ostream>`, so naively, it should say that you need that header.

But IWYU doesn't (at least, modulo bugs).  This is because of its attempt to analyze "author intent".

## Author intent ##

If code has `typedef Foo MyTypedef`, and you write `MyTypedef var;`, you are using `MyTypedef`, but are you also using `Foo`?  The answer depends on the _intent_ of the person who wrote the typedef.

In the `OutputEmitter` example above, while we don't know for sure, we can guess that the intent of the author was that clients should not be considered to use the underlying type -- and thus they shouldn't have to `#include <ostream>` themselves.  In that case, the typedef author takes responsibility for the underlying type, promising to provide all the definitions needed to make code compile.  The philosophy here is: "As long as you `#include "foo.h"`, you can use `OutputEmitter` however you want, without worry of compilation errors."

Some typedef authors have a different intent.  `<iosfwd>` has the line

    typedef basic_ostream<char> ostream;

but it does *not* promise "as long as you `#include <iosfwd>`, you can use `std::ostream` however you want, without worry of compilation errors."  For most uses of `std::ostream`, you'll get a compiler error unless you `#include <ostream>` as well.

So take a slightly modified version of the above `foo.h`:

    #include <iosfwd>
    typedef std::ostream OutputEmitter;

This is a self-contained .h file: it's perfectly legal to typedef an incomplete type (that's what `iosfwd` itself does).  But now IWYU had better tell `bar.cc` to `#include <ostream>`, or it will break the build.  The difference is in the author intent with the typedef.

Another case where author intent turns up is in function return types.  Consider this function declaration:

    Foo* GetSingletonObject();   // Foo is defined in foo.h

If you write `GetSingletonObject()->methodOnFoo()`, are you "using" `Foo::methodOnFoo`, such that you should `#include "foo.h"`?  Or are you supposed to be able to operate on the results of `GetSingletonObject` without needing to include the definition of the returned type?  The answer is: it depends on the author intent.  Sometimes the author is willing to provide the definition of the return type, sometimes it is not.

### Re-exporting ###

When the author of a file is providing a definition of a symbol from somewhere else, we say that the file is "re-exporting" that symbol.  In the first `OutputEmitter` example, we say that `foo.h` is re-exporting `ostream`.  As a result, people who `#include "foo.h"` get a definition of `ostream` along for free, even if they don't directly `#include <ostream>` themselves.  Another way of thinking about it is: if file A re-exports symbol B, we can pretend that A defines B, even if it doesn't.

(In an ideal world, we'd have a very fine-grained concept: "File A re-exports symbol S when it's used in the context of typedef T [or function F, or ...]," but in reality, we have the much looser concept "file A re-exports all symbols from file B.")

A more accurate include-what-you-use rule is this: "If you use a symbol, you must either `#include` the definition of the symbol, or `#include` a file that re-exports the symbol."

## Manual re-export identifiers ##

You can mark that one file is re-exporting symbols from another via an IWYU pragma in your source code:

    #include "private.h"   // IWYU pragma: export

This tells IWYU that if some other file uses symbols defined in `private.h`, they can `#include` you to get them, if they want.

The full list of IWYU pragmas is defined in [IWYUPragmas.md](IWYUPragmas.md).

## Automatic re-export ##

In certain situations, IWYU will decide that one file is exporting a symbol from another even without the use of a pragma.  These are places where the author intent is usually to re-export, such as with the `typedef` example above.  In each of these cases, a simple technique can be used to override IWYU's decision to re-export.

### Automatic re-export: typedefs ###

If you write

    typedef Foo MyTypedef;

IWYU has to decide whether your file should re-export `Foo` or not.  Here is how it gauges author intent:

* If you (the typedef author), directly `#include` the definition of the underlying type, then IWYU assumes you mean to re-export it.
* If you (the typedef author), explicitly provide a forward-declare of the underlying type, but do not directly `#include` its definition, then IWYU assumes you do not mean to re-export it.
* Otherwise, IWYU assumes you do not mean to re-export it.

For example:

    #include "foo.h"
    typedef Foo Typedef1;   // IWYU says you intend to re-export Foo

    class Bar;
    typedef Bar Typedef2;   // IWYU says you do not intend to re-export Bar

    #include "file_including_baz.h"   // does not define Baz itself
    typedef Baz Typedef3;   // IWYU says you do not intend to re-export Baz

If IWYU says you intend to re-export the underlying type, then nobody who uses your typedef needs to `#include` the definition of the underlying type.  In contrast, if IWYU says you do not intend to re-export the underlying type, then everybody who uses your typedef needs to `#include` the definition of the underlying type.

IWYU supports this in its analysis.  If you are using `Typedef1` in your code and `#include "foo.h"` anyway, IWYU will suggest you remove it, since you are getting the definition of `Foo` via the typedef.

### Automatic re-export: function return values ###

The same rule applies with the return value in a function declaration:

    #include "foo.h"
    Foo Func1();   // IWYU says you intend to re-export Foo

    class Bar;
    Bar Func2();   // IWYU says you do not intend to re-export Bar

    #include "file_including_baz.h"
    Baz Func3();   // IWYU says you do not intend to re-export Baz

(Note that C++ is perfectly happy with a forward-declaration of the return type, if the function is just being declared, and not defined.)

As of May 2011, the rule does *not* apply when returning a pointer or reference:

    #include "foo.h"
    Foo* Func1();   // IWYU says you do *not* intend to re-export Foo

    #include "bar.h"
    Bar& Func2();   // IWYU says you do *not* intend to re-export Bar

This is considered a bug, and the behavior will likely change in the future to match the case where the functions return a class.

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

In this case, IWYU will say that `baz.cc` does not need to `#include "foo.h"`, since `bar.h` re-exports it.

### Automatic re-export: conversion constructors ###

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

The above code does not compile, because the code to convert `11` to a `Foo` is not visible to `baz.cc`.  Either `baz.cc` or `bar.h` needs to `#include "foo.h"` to make the conversion constructor visible where `MyFunc` is being called.

The same rule applies as before:

    #include "foo.h"
    void Func1(Foo foo);   // IWYU says you intend to re-export Foo

    class Foo;
    void Func2(Foo foo);   // IWYU says you do not intend to re-export Foo

    #include "file_including_foo.h"
    void Func3(Foo foo);   // IWYU says you do not intend to re-export Foo

As before, if IWYU decides you do not intend to re-export `Foo`, then all callers (in this case, `baz.cc`) need to.

The rule here applies even to const references (which can also be automatically converted):

    #include "foo.h"
    void Func1(const Foo& foo);   // IWYU says you intend to re-export Foo
