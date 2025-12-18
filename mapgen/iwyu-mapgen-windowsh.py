#!/usr/bin/env python3

##===--- iwyu-mapgen-windowsh.py ------------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

"""Generate mappings for Win32 API headers.

Collects all headers included by Windows.h and remaps all of them to it.
"""

import re
import sys
import json
import argparse
from pathlib import Path
from collections.abc import Generator

IF_RE = re.compile(r"#\s*if")
ENDIF_RE = re.compile(r"#\s*endif")
LEAN_AND_MEAN1_RE = re.compile(r"#\s*ifndef\s+WIN32_LEAN_AND_MEAN")
LEAN_AND_MEAN2_RE = re.compile(r"#\s*if\s+!defined\s*\(WIN32_LEAN_AND_MEAN\)")

INCLUDE_RE = re.compile(r"#\s*include\s+<(.+)>")


HEADER_EXCLUSIONS = [
    # Standard C Library headers as of C23
    "assert.h",
    "complex.h",
    "ctype.h",
    "errno.h",
    "fenv.h",
    "float.h",
    "inttypes.h",
    "iso646.h",
    "limits.h",
    "locale.h",
    "math.h",
    "setjmp.h",
    "signal.h",
    "stdalign.h",
    "stdarg.h",
    "stdatomic.h",
    "stdbit.h",
    "stdbool.h",
    "stdckdint.h",
    "stddef.h",
    "stdint.h",
    "stdio.h",
    "stdlib.h",
    "stdmchar.h",
    "stdnoreturn.h",
    "string.h",
    "tgmath.h",
    "threads.h",
    "time.h",
    "uchar.h",
    "wchar.h",
    "wctype.h",
    # Textual headers
    "packon.h",
    "packoff.h",
    "pshpack1.h",
    "pshpack2.h",
    "pshpack4.h",
    "pshpack8.h",
    "poppack.h",
    # Intrinsics headers
    "intrin.h",
]


def parse_include_names(headerpath: Path) -> Generator[str]:
    """Parse the header file at headerpath and return all include names.

    This excludes headers that are guarded by the WIN32_LEAN_AND_MEAN macro,
    it's generally recommended that users should include those manually, so we
    don't want to map them to <Windows.h>.
    """

    if_count = 0
    lean_mean_idx: int | None = None

    with headerpath.open("r", encoding="utf-8") as fobj:
        for line in fobj.readlines():
            if IF_RE.search(line) is not None:
                # Check if we entered a WIN32_LEAN_AND_MEAN block
                if LEAN_AND_MEAN1_RE.search(line) is not None \
                   or LEAN_AND_MEAN2_RE.search(line) is not None:
                    lean_mean_idx = if_count

                if_count += 1
                continue

            if ENDIF_RE.search(line) is not None:
                if_count -= 1

                # Check if we escaped a WIN32_LEAN_AND_MEAN block
                if if_count == lean_mean_idx:
                    lean_mean_idx = None

                continue

            if lean_mean_idx is None:
                include_match = INCLUDE_RE.search(line)
                if include_match is not None \
                   and include_match.group(1) not in HEADER_EXCLUSIONS:
                    yield include_match.group(1)


def find_file_in(include_path: Path, filename: str) -> Path | None:
    found = list(include_path.rglob(filename, case_sensitive=False))
    if found:
        return found[0]

    return None


def fill_descendant_includes_of(
    into_set: set[str],
    include_root: Path,
    header_name: str,
) -> None:
    """Recursively find all includes of `header_name`.

    Found includes are added into the `into_set` set.
    """
    file = find_file_in(include_root, header_name)
    if file is None:
        return
    into_set.add(header_name)

    for include in parse_include_names(file):
        if include not in into_set:
            fill_descendant_includes_of(into_set, include_root, include)


def generate_imp_lines(include_names: list[str]) -> Generator[str]:
    """Generate a sequence of json-formatted strings in .imp format.

    This should ideally return a jsonable structure instead, and use json.dump
    to write it to the output file directly. But there doesn't seem to be a
    simple way to convince Python's json library to generate a "packed"
    formatting, it always prefers to wrap dicts onto multiple lines.

    Cheat, and use json.dumps for escaping each line.
    """

    def jsonline(mapping, indent):
        return (indent * " ") + json.dumps(mapping)

    for name in include_names:
        map_from = f"<{name}>"
        mapping = {"include": [map_from, "private", "<Windows.h>", "public"]}
        yield jsonline(mapping, indent=2)


def main(kit_path: Path) -> int:
    """Entry point."""

    if not kit_path.exists() or not find_file_in(kit_path, "Windows.h"):
        print(
            f"error: '{kit_path}' does not exist or does not contain Windows.h",
        )
        return 1

    accum_includes: set[str] = set()
    fill_descendant_includes_of(accum_includes, kit_path, "Windows.h")
    accum_includes.discard("Windows.h")
    accum_includes.discard("windows.h")

    print("[")
    print(",\n".join(generate_imp_lines(sorted(accum_includes))))
    print("]")

    return 0


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Generate IWYU mappings for the Win32 API."
    )
    parser.add_argument(
        "windows_kit_path",
        help="Path to a specific Windows Kits version"
        " (e.g. C:\\Program Files (x86)\\Windows Kits\\10\\Include\\<version>)",
    )
    args = parser.parse_args()
    sys.exit(main(Path(args.windows_kit_path)))
