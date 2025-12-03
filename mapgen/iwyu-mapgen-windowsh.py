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


# Standard C Library headers as of C23
STDLIB_EXCLUSIONS = [
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
]


accum_includes: set[str] = set()


def parse_include_names(headerpath: Path) -> Generator[str]:
    """
    Parse the header file at headerpath and return all include names.

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
                if (
                    LEAN_AND_MEAN1_RE.search(line) is not None
                    or LEAN_AND_MEAN2_RE.search(line) is not None
                ):
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
                if (
                    include_match is not None
                    and include_match.group(1) not in STDLIB_EXCLUSIONS
                ):
                    yield include_match.group(1)


def find_file_in(include_path: Path, filename: str) -> Path | None:
    found = list(include_path.rglob(filename))
    if found:
        return found[0]

    return None


def fill_descendant_includes_of(include_root: Path, header_name: str) -> None:
    """Recursively find all includes of `header_name`.

    Found includes are added into the `accum_includes` set.
    """
    file = find_file_in(include_root, header_name)
    if file is None:
        return
    accum_includes.add(header_name)

    for include in parse_include_names(file):
        if include not in accum_includes:
            fill_descendant_includes_of(include_root, include)


def find_latest_version(version_tuples: list[list[str]]) -> str:
    """Find the newest Windows Kits version from a list of available tuples.

    Windows Kits versioning follows a `<major.minor>.<build id>.<revision>`
    pattern, starting from Windows 8, the `<major.minor>` part is always `10.0`.
    So we pick the newest one based on the build id.
    """
    version_tuples.sort(key=lambda entry: entry[2], reverse=True)

    return ".".join(version_tuples[0])


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

    for name in sorted(include_names):
        # Regex-escape period and build a regex matching both "" and <>.
        map_from = f"<{name}>"
        mapping = {"include": [map_from, "private", "<Windows.h>", "public"]}
        yield jsonline(mapping, indent=2)


def main(windows_kits_path: Path) -> int:
    """Entry point."""

    include_path = windows_kits_path / "10" / "Include"

    if not include_path.exists():
        print(f"error: '{include_path}' is not a valid Windows Kits directory")

    available_versions: list[list[str]] = [
        entry.name.split(".", 4)
        for entry in include_path.iterdir()
        if entry.name.startswith("10")
    ]
    kit_root = include_path / find_latest_version(available_versions)

    if not kit_root.exists() or not find_file_in(kit_root, "Windows.h"):
        print(
            f"error: '{kit_root}' does not exist or does not contain Windows.h",
        )
        return 1

    fill_descendant_includes_of(kit_root, "Windows.h")
    accum_includes.remove("Windows.h")
    accum_includes.remove("windows.h")

    print("[")
    print(",\n".join(generate_imp_lines(sorted(accum_includes))))
    print("]")

    return 0


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Generate IWYU mappings for the Win32 API."
    )
    parser.add_argument(
        "windows_kits_path",
        help="Path to the 'Windows Kits' directory "
        "(e.g. C:\\Program Files (x86)\\Windows Kits\\)",
    )
    args = parser.parse_args()
    sys.exit(main(Path(args.windows_kits_path)))
