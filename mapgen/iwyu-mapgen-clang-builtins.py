#!/usr/bin/env python3

##===--- iwyu-mapgen-clang-builtins.py ------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

"""Generates mappings for Clang builtin headers.

The builtin headers serve as an interface between the compiler and libc. For
Clang, only a few of the builtin headers have private components and they are
related only by filename so that '__$public_$purpose.h' maps to '$public.h'.

Generate private-to-public mappings for double-underscore-prefixed names based
on this naming convention.
"""
import sys
import os
import re
import glob
import argparse
import json


IGNORED_DUNDER_PREFIXES = [
    # __clang prefix used for various proprietary and intrinsic support headers.
    "__clang_",
    # __wmmintrin prefix used for intrinsics.
    "__wmmintrin_"
]

OUTPUT_HEADER = f"""
Clang builtin mappings, generated with:
{" ".join(sys.argv)}
Do not edit!
""".strip()

# The capturing group pulls 'float' from '__float_header_macro.h', and similar.
HEADERNAME_RE = re.compile(r"^__([^_]+).*\.h$")


def print_output_header(comment_style):
    """ Print a "generated-by" header. """
    def prefix_lines(text, prefix):
        return prefix + ("\n" + prefix).join(text.splitlines())
    # Expand placeholders
    output_hdr = OUTPUT_HEADER.format()
    comment_prefix = comment_style + " "
    print(prefix_lines(output_hdr, comment_prefix))


def write_cxx_mappings(private_mappings):
    """ Write out mappings as C++ for IncludeMapEntry initialization. """
    print_output_header("//")
    print("// Private-to-public #include mappings.")
    for map_from, mapping_list in sorted(private_mappings.items()):
        for map_to in sorted(mapping_list):
            print("{ \"%s\", kPrivate, \"%s\", kPublic }," %
                  (map_from, map_to))


def write_imp_mappings(private_mappings):
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
    print("]")


def is_ignored(header_path):
    """ Check if the header is ignored by prefix. """
    headername = os.path.basename(header_path)
    for ignore in IGNORED_DUNDER_PREFIXES:
        if headername.startswith(ignore):
            return True
    return False


def create_mapping(header_path):
    """ Split header_path into a private and public include-name. """
    headername = os.path.basename(header_path)
    m = HEADERNAME_RE.match(headername)
    if not m:
        print("warning: unexpected headername format: %s" % headername,
              file=sys.stderr)
        return None, None
    private = m.group(0)
    public = m.group(1)
    return f"<{private}>", f"<{public}.h>"


def main(rootdir, lang):
    """ Entry point. """
    mappings = {}

    # Enumerate all dunder headers
    header_paths = glob.glob(os.path.join(rootdir, '__*.h'))
    for header_path in header_paths:
        if os.path.isdir(header_path):
            continue

        if is_ignored(header_path):
            continue

        private, public = create_mapping(header_path)
        if not private and not public:
            continue
        mappings.setdefault(private, set()).add(public)

    if lang == "c++":
        write_cxx_mappings(mappings)
    elif lang == "imp":
        write_imp_mappings(mappings)
    else:
        print("error: unsupported language: %s" % lang, file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--lang", choices=["c++", "imp"], default="imp",
                        help="output language")
    parser.add_argument("rootdir",
                        help=("Clang builtin header include root"))
    args = parser.parse_args()
    sys.exit(main(args.rootdir, args.lang))
