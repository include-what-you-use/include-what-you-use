#!/bin/bash

##===--- iwyu-dogfood.bash ------------------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

# This script is used to run IWYU over its own code base and produce a report
# suitable for GitHub pull requests.
# It assumes both include-what-you-use executable and compile_commands.json
# compilation database are present in the build directory specified.

if [ -z "$1" ]; then
    echo "usage: $0 <build-path>"
    exit 1
fi
builddir="$1"

# Generate a --check_also arg for every header-only module.
HEADER_ONLY=" \
iwyu_port.h \
iwyu_stl_util.h \
iwyu_string_util.h \
iwyu_use_flags.h \
iwyu_version.h \
"
check_alsos=$(for h in $HEADER_ONLY; do echo "-Xiwyu --check_also=*/$h"; done)

# Run IWYU over all source files using iwyu_tool.py with CMake-generated
# compilation database.
export IWYU_BINARY=$builddir/bin/include-what-you-use
./iwyu_tool.py -v -p "$builddir" *.cc -- -Xiwyu --error $check_alsos > iwyu-dogfood.out 2>&1
iwyu_exit=$?

# Apply changes in-tree using fix_includes.py.
./fix_includes.py --nosafe_headers --reorder < iwyu-dogfood.out \
                  > iwyu-dogfood.fix
fix_includes_exit=$?

# Let Git produce a diff of all suggested changes
git diff > iwyu-dogfood.diff

# Print out a GitHub Markdown result file.
cat<<EOF > iwyu-dogfood.md
### Informational: IWYU dogfood results

<details>
<summary>include-what-you-use (exit: $iwyu_exit)</summary>

\`\`\`
$(cat iwyu-dogfood.out)
\`\`\`

</details>

<details>
<summary>fix_includes.py (exit: $fix_includes_exit)</summary>

\`\`\`
$(cat iwyu-dogfood.fix)
\`\`\`

</details>

\`\`\`diff
$(cat iwyu-dogfood.diff)
\`\`\`

EOF
