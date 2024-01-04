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
check_alsos=$(for h in $HEADER_ONLY; do echo "-Xiwyu --check_also=*/$h"; done)

# Run IWYU over all source files using iwyu_tool.py with CMake-generated
# compilation database.
./iwyu_tool.py -p "$builddir" *.cc -- $check_alsos > iwyu-dogfood.out
iwyu_exit=$?

# Apply changes in-tree using fix_includes.py.
./fix_includes.py --nosafe_headers --reorder < iwyu-dogfood.out \
                  > iwyu-dogfood.fix
fix_includes_exit=$?

# Let git produce a diff of all suggested changes
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

<details>
<summary>diff</summary>
\`\`\`diff
$(cat iwyu-dogfood.diff)
\`\`\`
</details>
EOF
