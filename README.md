# Include What You Use #

[![Build Status](https://travis-ci.org/include-what-you-use/include-what-you-use.svg?branch=master)](https://travis-ci.org/include-what-you-use/include-what-you-use)

For more in-depth documentation, see http://github.com/include-what-you-use/include-what-you-use/tree/master/docs.


## Instructions for Users ##

"Include what you use" means this: for every symbol (type, function, variable, or macro) that you use in `foo.cc` (or `foo.cpp`), either `foo.cc` or `foo.h` should include a .h file that exports the declaration of that symbol. (Similarly, for `foo_test.cc`, either `foo_test.cc` or `foo.h` should do the including.)  Obviously symbols defined in `foo.cc` itself are excluded from this requirement.

This puts us in a state where every file includes the headers it needs to declare the symbols that it uses.  When every file includes what it uses, then it is possible to edit any file and remove unused headers, without fear of accidentally breaking the upwards dependencies of that file.  It also becomes easy to automatically track and update dependencies in the source code.


### CAVEAT ###

This is alpha quality software -- at best (as of February 2011).  It was written to work specifically in the Google source tree, and may make assumptions, or have gaps, that are immediately and embarrassingly evident in other types of code.  For instance, we only run this on C++ code, not C or Objective C.  Even for Google code, the tool still makes a lot of mistakes.

While we work to get IWYU quality up, we will be stinting new features, and will prioritize reported bugs along with the many existing, known bugs.  The best chance of getting a problem fixed is to submit a patch that fixes it (along with a unittest case that verifies the fix)!


### How to Build ###

Include-what-you-use makes heavy use of Clang internals, and will occasionally break when Clang is updated. Usually such discrepancies are detected by build bot and fixed promptly. The master branch follows Clang trunk.

We also have convenience tags and branches for released versions of Clang (called `clang_<version>`, e.g. `clang_5.0`). To build against a Clang release, check out the corresponding branch in IWYU before configuring the build. More details in the instructions below.

We support two build configurations: out-of-tree and in-tree.
 

#### Building out-of-tree ####

In an out-of-tree configuration, we assume you already have compiled LLVM and Clang headers and libs somewhere on your filesystem, such as via the `libclang-dev` package.

  * Create a directory for IWYU development, e.g. `iwyu-trunk`

  * Clone the IWYU Git repo:

        iwyu-trunk$ git clone https://github.com/include-what-you-use/include-what-you-use.git

  * Presumably, you'll be building IWYU with a released version of LLVM and Clang, so check out the corresponding branch. For example if you have Clang 3.2 installed, use the `clang_3.2` branch. IWYU `master` tracks LLVM & Clang trunk:

        iwyu-trunk$ cd include-what-you-use
        iwyu-trunk/include-what-you-use$ git checkout clang_3.2
        iwyu-trunk/include-what-you-use$ cd ..

  * Create a build root and use CMake to generate a build system linked with LLVM/Clang prebuilts. Note that CMake settings need to match exactly, so you may need to add `-DCMAKE_BUILD_TYPE=Release` or more to the command-line below:

        # This example uses the Makefile generator, but anything should work.
        iwyu-trunk$ mkdir build && cd build
        iwyu-trunk/build$ cmake -G "Unix Makefiles" -DIWYU_LLVM_ROOT_PATH=/usr/lib/llvm-3.4 ../include-what-you-use

  * Once CMake has generated a build system, you can invoke it directly from `build`, e.g.

        iwyu-trunk/build$ make

This configuration is more useful if you want to get IWYU up and running quickly without building Clang and LLVM from scratch.


#### Building in-tree ####

You will need the Clang and LLVM trees on your system, such as by [checking out](http://clang.llvm.org/get_started.html) their SVN trees (but don't configure or build before you've done the following.)

  * Clone the IWYU Git repo into the Clang source tree:

        llvm/tools/clang/tools$ git clone https://github.com/include-what-you-use/include-what-you-use.git

  * Edit `tools/clang/tools/CMakeLists.txt` and put in `add_subdirectory(include-what-you-use)`
  * Once this is done, IWYU is recognized and picked up by CMake workflow as described in the Clang Getting Started guide

This configuration is more useful if you're actively developing IWYU against Clang trunk. It's easier to set up correctly, but it requires that you build all of LLVM and Clang.


### How to Install ###

If you're building IWYU out-of-tree or installing pre-built binaries, you need to make sure it can find Clang built-in headers (`stdarg.h` and friends.)

Clang's default policy is to look in `path/to/clang-executable/../lib/clang/<clang ver>/include`. So if Clang 3.5.0 is installed in `/usr/bin`, it will search for built-ins in `/usr/lib/clang/3.5.0/include`.

Clang tools have the same policy by default, so in order for IWYU to analyze any non-trivial code, it needs to find Clang's built-ins in `path/to/iwyu/../lib/clang/3.5.0/include` where `3.5.0` is a stand-in for the version of Clang your IWYU was built against.

So for IWYU to function correctly, you need to copy in the Clang headers at a good location before running.

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
      CC="clang" CXX="clang++" cmake -DCMAKE_CXX_INCLUDE_WHAT_YOU_USE="path/to/iwyu;-Xiwyu;any;-Xwiyu;iwyu;-Xwiyu;args" ...

or, on Windows systems:

      mkdir build && cd build
      cmake -DCMAKE_CXX_COMPILER="%VCINSTALLDIR%/bin/cl.exe" -DCMAKE_CXX_INCLUDE_WHAT_YOU_USE="path/to/iwyu;-Xiwyu;any;-Xwiyu;iwyu;-Xwiyu;args" -G Ninja ...

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
