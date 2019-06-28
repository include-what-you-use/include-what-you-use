# Include What You Use #

[![Build Status](https://travis-ci.org/include-what-you-use/include-what-you-use.svg?branch=master)](https://travis-ci.org/include-what-you-use/include-what-you-use)

For more in-depth documentation, see [docs](docs).

## Instructions for Users ##

"Include what you use" means this: for every symbol (type, function, variable, or macro) that you use in `foo.cc` (or `foo.cpp`), either `foo.cc` or `foo.h` should include a .h file that exports the declaration of that symbol. (Similarly, for `foo_test.cc`, either `foo_test.cc` or `foo.h` should do the including.)  Obviously symbols defined in `foo.cc` itself are excluded from this requirement.

This puts us in a state where every file includes the headers it needs to declare the symbols that it uses.  When every file includes what it uses, then it is possible to edit any file and remove unused headers, without fear of accidentally breaking the upwards dependencies of that file.  It also becomes easy to automatically track and update dependencies in the source code.

### CAVEAT ###

This is alpha quality software -- at best (as of July 2018).  It was originally written to work specifically in the Google source tree, and may make assumptions, or have gaps, that are immediately and embarrassingly evident in other types of code.

While we work to get IWYU quality up, we will be stinting new features, and will prioritize reported bugs along with the many existing, known bugs.  The best chance of getting a problem fixed is to submit a patch that fixes it (along with a test case that verifies the fix)!

### How to Build ###

Include-what-you-use makes heavy use of Clang internals, and will occasionally break when Clang is updated. Usually such discrepancies are detected by build bot and fixed promptly. The master branch follows Clang trunk.

We also have convenience tags and branches for released versions of Clang (called `clang_<version>`, e.g. `clang_5.0`). To build against a Clang release, check out the corresponding branch in IWYU before configuring the build. More details in the instructions below.

We assume you already have compiled LLVM and Clang libraries on your system, either via packages for your platform or built from source. You can use this mapping table to combine Clang and IWYU versions correctly:

| Clang  | IWYU version | IWYU branch    |
|--------|--------------|----------------|
| 3.6    | 0.4          | `clang_3.6`    |
| 3.7    | 0.5          | `clang_3.7`    |
| 3.8    | 0.6          | `clang_3.8`    |
| 3.9    | 0.7          | `clang_3.9`    |
| 4.0    | 0.8          | `clang_4.0-r2` |
| 5.0    | 0.9          | `clang_5.0`    |
| 6      | 0.10         | `clang_6.0`    |
| 7      | 0.11         | `clang_7.0`    |
| 8      | 0.12         | `clang_8.0`    |
| 9      | 0.13         | `master`       |

> NOTE: If you use the Debian/Ubuntu packaging available from <https://apt.llvm.org>, you'll need the following packages installed:
>
> * `llvm-<version>-dev`
> * `libclang-<version>-dev`
> * `clang-<version>`
>
> Packaging for other platforms will likely be subtly different.

To set up an environment for building:

* Create a directory for IWYU development, e.g. `iwyu`

* Clone the IWYU Git repo:

      iwyu$ git clone https://github.com/include-what-you-use/include-what-you-use.git

* Presumably, you'll be building IWYU with a released version of LLVM and Clang, so check out the corresponding branch. For example, if you have Clang 6.0 installed, use the `clang_6.0` branch. IWYU `master` tracks LLVM & Clang trunk:

      iwyu$ cd include-what-you-use
      iwyu/include-what-you-use$ git checkout clang_6.0

* Create a build root and use CMake to generate a build system linked with LLVM/Clang prebuilts:

      # This example uses the Makefile generator, but anything should work.
      iwyu/include-what-you-use$ cd ..
      iwyu$ mkdir build && cd build

      # For IWYU 0.10/Clang 6 and earlier
      iwyu/build$ cmake -G "Unix Makefiles" -DIWYU_LLVM_ROOT_PATH=/usr/lib/llvm-6.0 ../include-what-you-use

      # For IWYU 0.11/Clang 7 and later
      iwyu/build$ cmake -G "Unix Makefiles" -DCMAKE_PREFIX_PATH=/usr/lib/llvm-7 ../include-what-you-use

  (substitute the `llvm-6.0` or `llvm-7` suffixes with the actual version compatible with your IWYU branch)

  or, if you have a local LLVM and Clang build tree, you can specify that as `CMAKE_PREFIX_PATH` for IWYU 0.11 and later:

      iwyu/build$ cmake -G "Unix Makefiles" -DCMAKE_PREFIX_PATH=/llvm-trunk/build ../include-what-you-use

* Once CMake has generated a build system, you can invoke it directly from `build`, e.g.

      iwyu/build$ make

Instructions for building Clang are available at <https://clang.llvm.org/get_started.html>.

### How to Install ###

If you're building IWYU out-of-tree or installing pre-built binaries, you need to make sure it can find Clang built-in headers (`stdarg.h` and friends.)

Clang's default policy is to look in `path/to/clang-executable/../lib/clang/<clang ver>/include`. So if Clang 3.5.0 is installed in `/usr/bin`, it will search for built-ins in `/usr/lib/clang/3.5.0/include`.

Clang tools have the same policy by default, so in order for IWYU to analyze any non-trivial code, it needs to find Clang's built-ins in `path/to/iwyu/../lib/clang/3.5.0/include` where `3.5.0` is a stand-in for the version of Clang your IWYU was built against.

Note that some distributions/packages may have different defaults, you can use `clang -print-resource-dir` to find the base path of the built-in headers on your system.

So for IWYU to function correctly, you need to copy the Clang `include` directory to the expected location before running (similarly, use `include-what-you-use -print-resource-dir` to learn exactly where IWYU wants the headers).

This weirdness is tracked in [issue 100](https://github.com/include-what-you-use/include-what-you-use/issues/100), hopefully we can make this more transparent over time.

### How to Run ###

The original design was built for Make, but a number of alternative run modes have come up over the years.

#### Plugging into Make ####

The easiest way to run IWYU over your codebase is to run

      make -k CXX=/path/to/llvm/Debug+Asserts/bin/include-what-you-use

or

      make -k CXX=/path/to/llvm/Release/bin/include-what-you-use

(include-what-you-use always exits with an error code, so the build system knows it didn't build a .o file.  Hence the need for `-k`.)

Include-what-you-use only analyzes .cc (or .cpp) files built by `make`, along with their corresponding .h files.  If your project has a .h file with no corresponding .cc file, IWYU will ignore it unless you use the `--check_also` switch to add it for analysis together with a .cc file.

#### Using with CMake ####

CMake has grown native support for IWYU as of version 3.3. See [their documentation](https://cmake.org/cmake/help/latest/prop_tgt/LANG_INCLUDE_WHAT_YOU_USE.html) for CMake-side details.

The `CMAKE_CXX_INCLUDE_WHAT_YOU_USE` option enables a mode where CMake first compiles a source file, and then runs IWYU on it.

Use it like this:

      mkdir build && cd build
      CC="clang" CXX="clang++" cmake -DCMAKE_CXX_INCLUDE_WHAT_YOU_USE="path/to/iwyu;-Xiwyu;any;-Xiwyu;iwyu;-Xiwyu;args" ...

or, on Windows systems:

      mkdir build && cd build
      cmake -DCMAKE_CXX_COMPILER="%VCINSTALLDIR%/bin/cl.exe" -DCMAKE_CXX_INCLUDE_WHAT_YOU_USE="path/to/iwyu;-Xiwyu;any;-Xiwyu;iwyu;-Xiwyu;args" -G Ninja ...

The option appears to be separately supported for both C and C++, so use `CMAKE_C_INCLUDE_WHAT_YOU_USE` for C code.

Note that with Microsoft's Visual C++ compiler, IWYU needs the `--driver-mode=cl` argument to understand the MSVC options from CMake.

#### Using with a compilation database ####

The `iwyu_tool.py` script predates the native CMake support, and works off the [compilation database format](https://clang.llvm.org/docs/JSONCompilationDatabase.html). For example, CMake generates such a database named `compile_commands.json` with the `CMAKE_EXPORT_COMPILE_COMMANDS` option enabled.

The script's command-line syntax is designed to mimic Clang's LibTooling, but they are otherwise unrelated. It can be used like this:

      mkdir build && cd build
      CC="clang" CXX="clang++" cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ...
      iwyu_tool.py -p .

or, on Windows systems:

      mkdir build && cd build
      cmake -DCMAKE_CXX_COMPILER="%VCINSTALLDIR%/bin/cl.exe" -DCMAKE_C_COMPILER="%VCINSTALLDIR%/VC/bin/cl.exe" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G Ninja ...
      python iwyu_tool.py -p .

Unless a source filename is provided, all files in the project will be analyzed.

See `iwyu_tool.py --help` for more options.

#### Applying fixes ####

We also include a tool that automatically fixes up your source files based on the IWYU recommendations.  This is also alpha-quality software!  Here's how to use it (requires python):

      make -k CXX=/path/to/llvm/Debug+Asserts/bin/include-what-you-use 2> /tmp/iwyu.out
      python fix_includes.py < /tmp/iwyu.out

If you don't like the way `fix_includes.py` munges your `#include` lines, you can control its behavior via flags. `fix_includes.py --help` will give a full list, but these are some common ones:

* `-b`: Put blank lines between system and Google includes
* `--nocomments`: Don't add the 'why' comments next to includes

### How to Correct IWYU Mistakes ###

* If `fix_includes.py` has removed an `#include` you actually need, add it back in with the comment '`// IWYU pragma: keep`' at the end of the `#include` line.  Note that the comment is case-sensitive.
* If `fix_includes.py` has added an `#include` you don't need, just take it out.  We hope to come up with a more permanent way of fixing later.
* If `fix_includes.py` has wrongly added or removed a forward-declare, just fix it up manually.
* If `fix_includes.py` has suggested a private header file (such as `<bits/stl_vector.h>`) instead of the proper public header file (`<vector>`), you can fix this by inserting a specially crafted comment near top of the private file (assuming you can write to it): '`// IWYU pragma: private, include "the/public/file.h"`'.

Current IWYU pragmas are described in [IWYUPragmas](docs/IWYUPragmas.md).
