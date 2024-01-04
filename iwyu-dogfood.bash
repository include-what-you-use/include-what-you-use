#!/bin/bash

builddir=$1
if [ -z "$builddir" ]; then
    echo "usage: $0 <build-path>"
    exit 1
fi

# Generate a --check_also arg for every header-only module.
HEADER_ONLY=" \
iwyu_port.h \
iwyu_stl_util.h \
iwyu_string_util.h \
iwyu_use_flags.h \
iwyu_version.h \
"

check_alsos=$(for hdr in $HEADER_ONLY; do echo "-Xiwyu --check_also=*/$hdr"; done)

# Run IWYU over all source files using iwyu_tool.py with CMake-generated
# compilation database.
./iwyu_tool.py -v -p "$builddir" *.cc -- \
               $check_alsos \
               >iwyu-check-iwyu.out
iwyu_exit=$?

# Apply changes in-tree using fix_includes.py.
./fix_includes.py --nosafe_headers --reorder < iwyu-check-iwyu.out \
                  >iwyu-check-iwyu.fix
fix_includes_exit=$?

# Print out a result file.
cat<<EOF >iwyu-check-iwyu.md
Informational: IWYU-on-IWYU results

<details>
<summary>include-what-you-use (exit: $iwyu_exit)</summary>
\`\`\`
$(cat iwyu-check-iwyu.out)
\`\`\`
</details>

<details>
<summary>fix_includes.py (exit: $fix_includes_exit)</summary>
\`\`\`
$(cat iwyu-check-iwyu.fix)
\`\`\`
</details>

<details>
<summary>diff</summary>
\`\`\`diff
$(git diff)
\`\`\`
</details>
EOF
