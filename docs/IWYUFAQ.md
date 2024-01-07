# IWYU FAQ #

IWYU is confusing at times, this document will try to straighten out some
question marks and provide practical advice.


## Why doesn't IWYU work with precompiled headers enabled? ##

> Running `include-what-you-use` fails with:
>
>     error: include-what-you-use does not support PCH
>
> Why?

IWYU does deep analysis of a compilation unit, both for relationship between
files, recording declarations and definitions across files, recording uses and
attributing uses to the header providing the best symbol definition. Being able
to identify which specific file (header) an entity lives in is central to this
work. IWYU relies on the Clang preprocessor library to provide the necessary
information.

When precompiled headers are enabled, a lot of the information simply
disappears. Precompiled headers are a build-time optimization, so the compiler
collapses most of the preprocessor activity into a pre-baked, reusable AST;
as far as we know there is no longer any record of `#include` directives or
transition between actual headers.

Rather than producing incomplete or inconsistent results, we simply reject
invocations that enable PCH using `-include-pch` or `/Yu` (in CL mode).

To get IWYU past this error, it's necessary to remove the PCH arguments from the
command line, but that implies the code must still compile after removing them.

There are some tricks for code bases that depend heavily on their PCH closure:

* If using `-include-pch`, replace the argument with `-include` of a
  corresponding header. It might also be necessary to delete any stray `.pch` or
  `.gch` files produced by earlier builds to stop IWYU from picking them up
  automatically.
* If using `/Yu` with `/FI` (MSVC's equivalent of `-include`), simply remove the
  `/Yu` argument to disable PCH but keep the include graph the same.
* If using `/Yu` _without_ `/FI`, the compiler expects that the first `#include`
  in every compilation unit is a placeholder for the PCH. This is how MSVC-only
  code bases typically use PCH. Remove `/Yu` and use `-Xiwyu --pch_in_code` to
  inform IWYU of this special role of the first `#include`.

Bonus question: how do precompiled headers relate to prefix headers?

A "prefix header" is the source for a "precompiled header". It's a regular
header file containing `#include` directives for any expensive-to-parse headers
we want to bundle into a precompiled header. Clang, GCC and MSVC all have
command-line arguments to compile the prefix header into binary format, at which
point it becomes a precompiled header. Usually when the prefix header is
included (whether in source code using `#include` or from the command-line using
`-include`) the compiler magically skips it and replaces it with the precompiled
header.

(Historical note: there used to be a strong convention in MSVC code bases to
call the prefix header `stdafx.h`, because that's what the `New C++ File...` UI
would default to.)

That should paint a background to help explain the following IWYU switches:

* `-Xiwyu --pch_in_code` does not mean IWYU understands precompiled headers. It
  means the prefix header is included explicitly in code and should be treated
  more or less as if it was specified with `-include` or `/FI` (i.e. cannot be
  moved or removed).
* `-Xiwyu --prefix_header_includes` tells IWYU how much to respect the prefix
  header; it can either add, remove or keep headers for source files, when they
  are also present in the prefix header.

The `-Xiwyu --prefix_header_includes` switch defaults to `add`, which helps
extract dependencies from prefix header/PCH into the source files that actually
use them.
