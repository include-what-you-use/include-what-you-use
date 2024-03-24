# IWYU mappings #

One of the difficult problems for IWYU is distinguishing between which header contains a symbol definition and which header is the actual documented header to include for that symbol.

For example, in GCC's libstdc++, `std::unique_ptr<T>` is defined in `<bits/unique_ptr.h>`, but the documented way to get it is to `#include <memory>`.

Another example is `NULL`. Its authoritative header is `<cstddef>`, but for practical purposes `NULL` is more of a keyword, and according to the standard it's acceptable to assume it comes with `<cstring>`, `<clocale>`, `<cwchar>`, `<ctime>`, `<cstdio>` or `<cstdlib>`. In fact, almost every standard library header pulls in `NULL` one way or another, and we probably shouldn't force people to `#include <cstddef>`.

To simplify IWYU deployment and command-line interface, many of these mappings are compiled into the executable. These constitute the *default mappings*.

However, many mappings are toolchain- and version-dependent. Symbol homes and `#include` dependencies change between releases of GCC and are dramatically different for the standard libraries shipped with Microsoft Visual C++. Also, mappings such as these are usually necessary for third-party libraries (e.g. Boost, Qt) or even project-local symbols and headers as well.

Any mappings outside of the default set can therefore be specified as external *mapping files*.

## Default mappings ##

IWYU's default mappings are hard-coded in `iwyu_include_picker.cc`, and are very GCC-centric. There are both symbol- and include mappings for GNU libstdc++ and libc.

## Mapping files ##

The mapping files conventionally use the `.imp` file extension, for "Iwyu MaPping" (terrible, I know). They have the following general form:

    [
      { <directive>: <data> },
      { <directive>: <data> }
    ]

Directives can be one of the literal strings:

* `include`
* `symbol`
* `ref`

and data varies between the directives, see below.

Note that you can mix directives of different kinds within the same mapping file.

The `.imp` format looks like [JSON](http://json.org/), but IWYU actually uses LLVM's [YAML](https://yaml.org/) parser to interpret the mapping files, which technically allows a richer syntax. We try to use a minimum of YAML features to get basic functionality (trailing comma syntax, `#` comments), but please be conservative to keep it easy to do machine rewrites.

### Include mappings ###

The `include` directive specifies a mapping between two include names (relative path, including quotes or angle brackets.)

This is typically used to map from a private implementation detail header to a public facade header, such as our `<bits/unique_ptr.h>` to `<memory>` example above.

Data for this directive is a list of four strings containing:

* The include name to map from
* The visibility of the include name to map from
* The include name to map to
* The visibility of the include name to map to

For example;

    { "include": ["<bits/unique_ptr.h>", "private", "<memory>", "public"] }

Most of the original mappings were generated with shell scripts (as evident from the embedded comments) so there are several multi-step mappings from one private header to another, to a third and finally to a public header. This reflects the `#include` chain in the actual library headers. A hand-written mapping could be reduced to one mapping per private header to its corresponding public header.

Include mappings support a special wildcard syntax for the first entry:

    { "include": ["@<internal/.*>", "private", "<public>", "public"] }

The `@` prefix is a signal that the remaining content is a regex, and can be used to re-map a whole subdirectory of private headers to a public facade header.

The `include-what-you-use` program has a `--regex` argument to select dialect;

* `llvm`: a basic, fast implementation (default)
* `ecmascript`: a more capable, slower implementation with support for e.g. negative lookaround

The performance hit of `ecmascript` can be quite significant for large mapping files with many regex patterns, so mind your step.

### Symbol mappings ###

The `symbol` directive maps from a qualified symbol name to its authoritative header.

Data for this directive is a list of four strings containing:

* The symbol name to map from
* The visibility of the symbol
* The include name to map to
* The visibility of the include name to map to

For example;

    { "symbol": ["NULL", "private", "<cstddef>", "public"] }

The symbol visibility is largely redundant -- it must always be `private`. It isn't entirely clear why symbol visibility needs to be specified, and it might be removed moving forward.

Unlike `include`, `symbol` directives do not support the `@`-prefixed regex syntax in the first entry. Track the [following bug](https://github.com/include-what-you-use/include-what-you-use/issues/233) for updates.

### Mapping refs ###

The last kind of directive, `ref`, is used to pull in another mapping file, much like the C preprocessor's `#include` directive. Data for this directive is a single string: the filename to include.

For example;

    { "ref": "more.symbols.imp" },
    { "ref": "/usr/lib/other.includes.imp" }

The rationale for the `ref` directive was to make it easier to compose project-specific mappings from a set of library-oriented mapping files. For example, IWYU might ship with mapping files for [Boost](http://www.boost.org), the SCL, various C standard libraries, the Windows API, the [Poco Library](http://pocoproject.org), etc. Depending on what your specific project uses, you could easily create an aggregate mapping file with refs to the relevant mappings.

### Command-line switches for mapping files ###

Mapping files are specified on the command-line using the `--mapping_file` switch:

    $ include-what-you-use -Xiwyu --mapping_file=foo.imp some_file.cc

The switch can be added multiple times to add more than one mapping file.

If the mapping filename is relative, it will be looked up relative to the current directory.

`ref` directives are first looked up relative to the current directory and if not found, relative to the referring mapping file.

The default mappings can be turned off (e.g. for baremetal projects like an OS kernel) using the `--no_default_mappings` switch:

    $ include-what-you-use -Xiwyu --no_default_mappings --mapping_file=kernel_libc.imp kernel/main.c
