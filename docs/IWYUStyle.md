# IWYU Coding Style #

IWYU is developed by a small number of loosely knit maintainers, and we rely on
external contributions to stand a chance to improve the tool.

We use code reviews as a stage gate for every patch, via the GitHub pull request
system. We try our best to make reviews constructive and avoid power struggles,
so please accept feedback in the good faith it was delivered.

This document describes the most important of the conventions that contributors
are expected to follow, so as to help make code reviews more productive and the
code base more consistent and maintainable.

## Git workflow ##

We prefer a linear history on the master branch, so we use a rebase workflow and
avoid merge commits.

To post a pull request:

* Fork the `include-what-you-use` repository on GitHub
* Clone your fork
* Create a topic branch in your clone
* Make changes
* Push branch to your fork
* Create pull request against the main repo

To address review feedback:

* Make changes in your branch
* Rewrite history in your branch to the way you want the final commit series. No
  use to keep incremental review feedback as separate commits; we're only
  interested in what the change set as a whole will look like.
* Make sure your branch is rebased on top of the latest upstream master and all
  tests  pass
* Force-push to your fork. This will update the pull request for the next round
  of review.

Note that it's fine to rewrite history of throw-away branches such as pull
request branches. If you need a more stable history for some reason (sharing
with other developers, tagging, etc), use a dedicated branch for the pull
request so you're free to rebase and amend it based on review feedback.

There's no strong convention or tooling for commit messages (yet), but please
see https://chris.beams.io/posts/git-commit/ for set of guidelines to make the
Git log a nice place to visit.

## License headers ##

Every source file must have a license header on the form:

```
//===--- <filename> - <description> ---------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
```

(With the corresponding comment prefix/suffix for other languages.)

There is a utility script, `iwyu-check-license-header.py` that can be used to
automatically add a license header with the right filename to a new source file.

## C++ ##

IWYU originated within Google, so its basic style is based on what Google had in
their style guide at that time. Please refer to the [Google C++ style guide][1]
for details, but note that newer decisions may not apply to IWYU. The most
important points:

* At most 80 characters per line
* 2 spaces indentation, 4 spaces for continued lines
* Unix line endings
* PascalCase for types and functions, hacker_case for variables,
  LOUDMOUTH_CASE for macros
* Pointer asterisk and reference ampersand binds to the type name, not the
  declared variable (i.e. declarations look like `Foo* f;` and `Bar& b;`)
* When in doubt, mimic the code around you.

IWYU has a `.clang-format` file, so everything should play out nicely if you use
clang-format on your edited lines. PLEASE do not reformat more than lines you
touch, especially not whole files. The git-clang-format tool can help with this.

We observe a few [LLVM-inspired rules][2] for IWYU:

* **Use 'auto' sparingly** -- we don't subscribe to the "Almost always auto"
guideline in IWYU. In line with LLVM tradition we use `auto` when it improves
readability, and take that to mean when the type is already clearly visible on
the line (e.g. `const auto& d = cast<Derived>(b);`) or when iterator
declarations would grow too unwieldy (e.g. `const auto i = some_map.begin();`).

* **Prefer 'static' to anonymous namespaces** -- while `static` is decidedly a
C-ism, we find it's a readability win compared to anonymous namespaces to mark
functions for internal linkage. The LLVM coding standards make the argument
well, so no use to repeat it here.

Other than that, we mostly follow mainstream C++ style, if there is such a
thing.


## Python ##

Older IWYU Python code follows the [Google convention for Python code][3], but
newer scripts typically just use [PEP8][4].

Again, mimic the code around you.


## Regression tests ##

IWYU uses a regression testing framework purpose-built to verify IWYU
diagnostics.

There are a few conventions for IWYU testcases we use to keep the test suite
maintainable and effective. Note that we have many tests breaking these
guidelines, but that's considered a bug that we will work on over time.

### Testcase per construct ###

IWYU started out with one single testcase -- `badinc.cc` -- to cover all aspects
of IWYU. This has proved hard to maintain, as many assertions are subtly
interconnected and it can be difficult to map a test failure back to the feature
that triggered it.

Now that IWYU handles most of the basics, we try to add new testcases organized
by the language construct or IWYU feature they cover. This tends to produce a
large number of testcases, and we'll probably have to go back and organize them
further at some point.

Some things to keep in mind:

* Keep testcase filenames short -- this is a case of "as short as possible, but
  no shorter". Every file has its name in the license header, the stem is
  repeated for supporting headers, and testcase names occasionally need to be
  typed without completion, so it's nice if they're short and sweet. But they
  also need to be meaningful. Find the balance.
* If a change pertains to a feature covered by an existing testcase, consider
  extending it instead of creating a new one.
* Don't use another testcase's supporting headers; that creates unwanted
  coupling between tests. But see below for a set of headers we do encourage
  sharing.

### Reusable headers ###

Prefer the reusable headers for simple constructs.

The test stack contains a few headers aimed for reuse:

* `indirect.h` -- defines `IndirectClass`, a very basic C++ class
* `direct.h` -- includes `indirect.h`
* `subdir/indirect_subdir.h` -- defines `IndirectSubDirClass`
* `subdir/direct_subdir.h` -- includes `subdir/indirect_subdir.h`
* `subdir/dotdot_indirect.h` -- includes `../indirect.h`

If you need a basic class as a return type, argument type, variable or template
argument, `IndirectClass` should be your preference.

The `direct*.h` variants are used to force IWYU to do its analysis and
replacement; it will suggest that you remove `direct.h` and include `indirect.h`
directly. More on this below.

The `subdir/` headers can be used to test that IWYU is not confused by directory
structure (which has been known to happen).

### Avoid system headers ###

Avoid using system headers for regression tests.

It's tempting to phrase your testcase in terms of the library that triggered a
bug. But our test runners do not have all libraries installed. The C++ standard
library is a non-obvious example of this: it's always available, but IWYU aims
to be portable, and standard library _implementations_ differ wildly.

For example, if `std::map` triggers a bad behavior with IWYU on your MSVC system
because it uses some template trick in its implementation, it's likely libstdc++
or libc++ do not use the same trick, and exhibit different behavior.

Likewise, the internal physical design details of standard library
implementations are usually wildly different -- libstdc++ has all its private
headers in a directory called `bits/`, MSVC prefixes all private library headers
with `x`, e.g. `xstring`, etc. IWYU uses mappings to overcome these differences.

Since we can't control what standard library IWYU will be tested with, we prefer
to avoid standard library types and headers as far as possible, and instead try
to reduce constructs that cause problems to a minimal mockup. Clang takes a
similar approach in its test suite.

Note that many of IWYU's older tests *do* use system headers, and as a
consequence we have incidental failures on many non-Linux platforms.

### Fail first ###

When making changes, make sure your testcases fail without your changes, and
pass with them. This is an easy way to make sure your tests cover the right
thing.

### Force diagnostics ###

Use IWYU diagnostics for more fine-grained testing.

It's tempting to write a test such that it satisfies all IWYU rules and produces
a `(x.cc has correct #includes/fwd-decls)` message.

But it's usually better to use the reusable headers to force some churn. Besides
the summary output at the end ("should add/should remove/full include list"),
IWYU also emits helpful diagnostics at the point of every use, e.g.

* `warning: X is defined in "tests/cxx/x.h", which isn't directly #included`
* `warning: X needs a declaration, but does not provide or directly #include...`

The test framework has native pattern matching support for these diagnostics, to
make it easier to check that IWYU triggers where expected and doesn't suggest
keeping a header for some unrelated reason.

Therefore, prefer to include `direct.h` when you need `IndirectClass` (or the
corresponding per-testcase header) to force a diagnostic on use.

### Header names ###

Where the reusable headers are not enough, use per-test headers with a special
naming convention.

In order to be able to force diagnostics, it's typically best to split
supporting headers into a direct and indirect pair. For a testcase named `x.cc`,
these would be named:

* `x-indirect.h` for a header containing symbol declarations
* `x-direct.h` for a header including `x-indirect.h`

If you need more than one header, use `-i<N>` and `-d<N>` for suffixes:

* `x-i1.h` and `x-i2.h` for headers containing symbol declarations
* `x-d1.h` for a header including `x-i1.h`
* `x-d2.h` for a header including `x-i2.h`

Depending on the complexity of your testcase, you can usually use a single
`-direct.h` header for multiple `-i<N>.h` headers.

It's a good idea to keep the number of headers in a single testcase small, but
some tests require more.

Also, beware of the associated header -- if you use a header `x.h` for testcase
`x.cc`, IWYU will identify it as an associated header and analysis will be
slightly different. That can be used to simplify testing, but it can also
produce results that aren't necessarily the same for non-associated headers.

### Symbol names ###

Name symbols after their role or what sets them apart in the test.

And from the other perspective: don't name symbols for testing after their
containing header file.

If it's important that a type is a template, name it `Template`. If the defining
trait of a class from IWYU's perspective is that it's nested in another class,
name it `Nested`, etc.

With the direct/indirect naming convention for headers, attempting to match up
symbol and header names turns into an exercise in obscurity, and we prefer clear
and communicative names.

Keeping strictly separate naming schemes for test symbols and headers also makes
it easier to rename things without throwing Git's content tracking for a loop.

### Command-line arguments ###

If a test needs to pass additional command-line args to IWYU, they are coded
into `run_iwyu_tests.py`. There are three flag mappings:

* `flags_map` -- for IWYU flags, e.g. `--no_comments` (automatically prefixed
  with `-Xiwyu`)
* `clang_flags_map` -- for Clang flags, e.g. `-std=c++17`
* `include_map` -- additional header search paths, e.g. `subdir/` (automatically
  prefixed with `-I`)

The maps are all keyed by test file basename.


[1]: https://google.github.io/styleguide/cppguide.html
[2]: https://llvm.org/docs/CodingStandards.html
[3]: https://google.github.io/styleguide/pyguide.html
[4]: https://www.python.org/dev/peps/pep-0008
