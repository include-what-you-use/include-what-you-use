#!/usr/bin/env python3

##===--- iwyu-mapgen-clang-intrin.py --------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

"""Generates mappings for Clang intrinsics headers.

The Clang intrinsics have a simple rule: each private header contains an #error
directive which points to the public header:

    #if !defined X86GPRINTRIN_H_
    #error "Never use <bmiintrin.h> directly; include <x86gprintrin.h> instead."
    #endif

Scan over all intrinsics headers, match the #error messages and build mappings.
"""
import sys
import os
import re
import glob
import argparse
import json
import textwrap


# In addition to the static patterns in the intrinsics headers, there are also
# some known public-to-public mappings where public headers are interchangeable.
# Keep a curated mapping.
KNOWN_PUBLIC_MAPPINGS = {
    # Allow Intel intrinsics umbrella header.
    "<emmintrin.h>": {"<immintrin.h>"},
    "<mmintrin.h>": {"<immintrin.h>"},
    "<nmmintrin.h>": {"<immintrin.h>"},
    "<pmmintrin.h>": {"<immintrin.h>"},
    "<popcntintrin.h>": {"<immintrin.h>"},
    "<smmintrin.h>": {"<immintrin.h>"},
    "<tmmintrin.h>": {"<immintrin.h>"},
    "<wmmintrin.h>": {"<immintrin.h>"},
    "<x86gprintrin.h>": {"<immintrin.h>"},
    "<xmmintrin.h>": {"<immintrin.h>"},
}

# Sometimes the intrinsics headers misspell a header in a "Never" comment;
# resolve these on a case-by-case basis.
KNOWN_CLANG_TYPOS = {
    # This has been fixed in https://github.com/llvm/llvm-project/pull/207507.
    "<avx512vp2intersect.h>": "<avx512vp2intersectintrin.h>",
}

# Observed variations:
# "Never use <X> directly; include <A> instead."
# "Never use <X> directly; use <A> instead."
# Match any single word after "Never" and any single word after "directly;".
MESSAGE_RE = re.compile(
    r'"Never \w+ (<[^>]+>) directly; \w+ (<[^>]+>) instead.*"'
)

# Match C line continuations, and surrounding whitespace, e.g. these two lines:
#
#    #error                     \
#       "Hello world!"
#
# will match the entire range:
#    #error[                    \
#       ]"Hello world!"
#
# so we can collapse it to a single space.
LINE_CONTINUATION_RE = re.compile(r"\s*\\[ \t]*\r?\n\s*")


OUTPUT_HEADER = f"""
Clang intrinsics mappings, generated with:
{" ".join(sys.argv)}
Do not edit!
""".strip()


def print_output_header(comment_style):
    """ Print a "generated-by" header. """
    def prefix_lines(text, prefix):
        return prefix + ("\n" + prefix).join(text.splitlines())
    # Expand placeholders
    output_hdr = OUTPUT_HEADER.format()
    comment_prefix = comment_style + " "
    print(prefix_lines(output_hdr, comment_prefix))


def write_cxx_mappings(private_mappings, public_mappings):
    """ Write out mappings as C++ for IncludeMapEntry initialization. """
    print_output_header("//")
    print("// Private-to-public #include mappings.")
    for map_from, mapping_list in sorted(private_mappings.items()):
        for map_to in sorted(mapping_list):
            print("{ \"%s\", kPrivate, \"%s\", kPublic }," %
                  (map_from, map_to))

    print("// Public-to-public #include mappings.")
    for map_from, mapping_list in sorted(public_mappings.items()):
        for map_to in sorted(mapping_list):
            print("{ \"%s\", kPublic, \"%s\", kPublic }," %
                  (map_from, map_to))


def write_imp_mappings(private_mappings, public_mappings):
    """ Write out mappings as YAML for .imp mappings. """
    def quoted(name):
        return json.dumps(name)

    print_output_header("#")
    print("[")
    print("  # Private-to-public #include mappings.")
    for map_from, mapping_list in sorted(private_mappings.items()):
        for map_to in sorted(mapping_list):
            print('  { "include": [%s, "private", %s, "public"] },' %
                  (quoted(map_from), quoted(map_to)))

    print("  # Public-to-public #include mappings.")
    for map_from, mapping_list in sorted(public_mappings.items()):
        for map_to in sorted(mapping_list):
            print('  { "include": [%s, "public", %s, "public"] },' %
                  (quoted(map_from), quoted(map_to)))
    print("]")


def parse_header(path):
    """ Return list of (private, public) tuples found in one header. """
    with open(path, encoding="utf-8", errors="replace") as f:
        # Rejoin continued lines as the preprocessor does (avx512vlfp16intrin.h
        # et al. split the #error directive across lines).
        text = LINE_CONTINUATION_RE.sub(" ", f.read())

    res = []
    for m in MESSAGE_RE.finditer(text):
        private = m.group(1).strip()
        public = m.group(2).strip()
        res.append((private, public))
    return res


def main(rootdir, lang):
    """ Entry point. """
    mappings = {}

    # Don't recurse; headers in ppc_wrappers/ shadow the main ones in rootdir.
    header_paths = glob.glob(os.path.join(rootdir, '*.h'))
    for header_path in header_paths:
        if os.path.isdir(header_path):
            continue

        for private, public in parse_header(header_path):
            # Resolve known typos.
            private = KNOWN_CLANG_TYPOS.get(private, private)
            mappings.setdefault(private, set()).add(public)

    if lang == "c++":
        write_cxx_mappings(mappings, KNOWN_PUBLIC_MAPPINGS)
    elif lang == "imp":
        write_imp_mappings(mappings, KNOWN_PUBLIC_MAPPINGS)
    else:
        print("error: unsupported language: %s" % lang, file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--lang", choices=["c++", "imp"], default="imp",
                        help="output language")
    parser.add_argument("rootdir",
                        help=("Clang intrinsics include root"))
    args = parser.parse_args()
    sys.exit(main(args.rootdir, args.lang))
