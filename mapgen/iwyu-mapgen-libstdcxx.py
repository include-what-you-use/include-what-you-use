#!/usr/bin/env python3

##===--- iwyu-mapgen-libstdcxx.py -----------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

"""Generates mappings for libstdc++ headers.

The GNU libstdc++ standard library has fairly strong conventions for private
vs. public headers:

- The library is split into a portable part in /usr/include/c++ and a
  target-specific part in /usr/include/$target/c++
- Private headers with a designated public header usually use a Doxygen
  @headername directive to say which public header should be used instead
- Inline reusable template code is in .tcc files, all considered private
- Most private headers are in conventionally named subdirectories (bits/,
  detail/ or debug/)

IWYU dynamically maps @headername directives, so we don't need to (and
shouldn't) generate mappings for them. But we can use the presence of
@headername to decide this is a private header with an unambiguous public
mapping.

For all other headers, we look at #include directives and map backwards from any
private header to any public header that includes it.

To handle transitive mappings, we also map from any private header to any other
private header that includes it, except for the ones already mapped to public.
"""

import argparse
import json
import os
import re
import sys
import textwrap


OUTPUT_HEADER = """
GNU libstdc++ mappings generated with:

%s

Do not edit!
"""

IGNORE_HEADERS = frozenset((
    # These internal headers are just textual includes to generate
    # warnings. They do not define any symbols, so ignore them for mappings.
    "backward/backward_warning.h",
    "bits/c++0x_warning.h",
))

# These private headers are included by multiple public headers, but should
# always map to a single one.
EXPLICIT_MAPPINGS = {
    "bits/exception.h": "exception",
    # Only ambiguous in libstdc++-14, but override always.
    "debug/vector": "vector",
}

class Header:
    """ Carries information about a single libstdc++ header. """
    def __init__(self, includename, has_headername, includes):
        self.includename = includename
        self.has_headername = has_headername
        self.includes = includes

    @classmethod
    def parse(cls, path, includename):
        """ Parse a single file into a Header. """
        with open(path, "r") as fobj:
            text = fobj.read()

        # Some private headers use Doxygen directive '@headername{xyz}' to
        # indicate which is the public header.
        has_headername = bool(re.search(r".*@headername{.*}", text))

        # Parse all #include directives
        included_names = re.finditer(r'^\s*#\s*include\s*["<](.*)[">]',
                                     text, re.MULTILINE)
        includes = [m.group(1) for m in included_names]

        return Header(includename, has_headername, includes)

    def is_private(self):
        """ Return True if this Header has any private indicator. """
        # If the file contains @headername directives, it is a private header.
        if self.has_headername:
            return True
        # All .tcc files are private.
        if self.includename.endswith(".tcc"):
            return True
        # All debug/ headers are private.
        if self.includename.startswith("debug/"):
            return True
        # The Policy-Based Data Structures ext library has all its private
        # headers in detail/.
        if self.includename.startswith("ext/pb_ds/detail/"):
            return True

        # All headers immediately under bits/ are private.
        dirpath = os.path.dirname(self.includename)
        lastdir = os.path.basename(dirpath)
        if lastdir == "bits":
            return True

        return False


def shell_wrap(argv, width):
    """ Wrap a shell command to width with proper line continuation chars
    (assumes no quoted arguments with spaces).
    """
    # Remove 2 chars for potential line continuation.
    width -= 2

    # Wrap the command text as a single paragraph.
    command_text = " ".join(argv)
    wrapped = textwrap.wrap(command_text, width=width, break_long_words=False,
                            break_on_hyphens=False, initial_indent="",
                            subsequent_indent="    ")

    # Add line continuation for all lines except last.
    wrapped = [line + " \\" for line in wrapped[:-1]] + [wrapped[-1]]
    return "\n".join(wrapped)


def output_header(comment_prefix):
    """Return a header comment containing the exact command invocation, wrapped
    to column width and commented with a prefix of choice.
    """
    # Comment prefix will occupy character(s) + one space.
    width = 80 - len(comment_prefix) + 1

    # Write the argv into the header, nicely wrapped.
    hdrtext = OUTPUT_HEADER.strip() % shell_wrap(sys.argv, width)

    def prefix(line):
        """ Prefix each line with comment chars (and space if non-empty) """
        if not line:
            return comment_prefix
        return comment_prefix + " " + line

    hdrlines = [prefix(line) for line in hdrtext.splitlines()]
    return "\n".join(hdrlines)


def write_cxx_mappings(public_mappings, private_mappings):
    """ Write out mappings as C++ for pasting into iwyu_include_picker.cc. """
    print(output_header("//"))
    print("const IncludeMapEntry libstdcpp_include_map[] = {")
    print("  // Private-to-public #include mappings.")
    for map_from, mapping_list in sorted(public_mappings.items()):
        for map_to in sorted(mapping_list):
            print("  { \"<%s>\", kPrivate, \"<%s>\", kPublic }," %
                  (map_from, map_to))

    print("  // Private-to-private #include mappings.")
    for map_from, mapping_list in sorted(private_mappings.items()):
        for map_to in sorted(mapping_list):
            print("  { \"<%s>\", kPrivate, \"<%s>\", kPrivate }," %
                  (map_from, map_to))
    print("};")


def write_imp_mappings(public_mappings, private_mappings):
    """ Write out mappings as YAML for .imp mappings. """
    def quoted(name):
        return json.dumps("<%s>" % name)

    print(output_header("#"))
    print("[")
    print("  # Private-to-public #include mappings.")
    for map_from, mapping_list in sorted(public_mappings.items()):
        for map_to in sorted(mapping_list):
            print('  { "include": [%s, "private", %s, "public"] },' %
                  (quoted(map_from), quoted(map_to)))

    print("  # Private-to-private #include mappings.")
    for map_from, mapping_list in sorted(private_mappings.items()):
        for map_to in sorted(mapping_list):
            print('  { "include": [%s, "private", %s, "private"] },' %
                  (quoted(map_from), quoted(map_to)))
    print("]")


def main(rootdirs, lang, verbose):
    """ Entry point. """
    public_headers = {}
    private_headers = {}

    # Collect all headers.
    for rootdir in rootdirs:
        for root, dirs, files in os.walk(rootdir):
            for name in files:
                headerpath = os.path.join(root, name)
                includename = os.path.relpath(headerpath, rootdir)
                if includename in IGNORE_HEADERS:
                    continue

                header = Header.parse(headerpath, includename)
                if header.is_private():
                    private_headers[header.includename] = header
                else:
                    public_headers[header.includename] = header

    # There must be no overlap between public and private headers.
    assert public_headers.keys().isdisjoint(private_headers.keys())

    # Build private-to-public mappings for all private headers without
    # @headername included by a public header.
    raw_public_mappings = {}
    for header in public_headers.values():
        for include in header.includes:
            included_header = private_headers.get(include)
            if included_header and not included_header.has_headername:
                raw_public_mappings.setdefault(include, set()).add(
                    header.includename)

    # Overwrite any explicit mappings.
    public_mappings = {}
    for private, public in raw_public_mappings.items():
        override = EXPLICIT_MAPPINGS.get(private)
        if override:
            public_mappings[private] = {override}
        else:
            public_mappings[private] = public

    # Keep only unambiguous mappings.
    public_mappings = {k: v for k, v in public_mappings.items() if len(v) == 1}

    # Print suppressed mappings in verbose mode.
    if verbose:
        for private, public in raw_public_mappings.items():
            if private in public_mappings:
                continue
            print("suppressed ambiguous mapping: %s -> %s" %
                  (private, public), file=sys.stderr)

    # Then add private-to-private mappings for all private headers including
    # another private header.
    private_mappings = {}
    for header in private_headers.values():
        for include in header.includes:
            included_header = private_headers.get(include)
            if included_header and not included_header.has_headername:
                private_mappings.setdefault(include, set()).add(
                    header.includename)

    # Write out format depending on --lang switch
    if lang == "c++":
        write_cxx_mappings(public_mappings, private_mappings)
    elif lang == "imp":
        write_imp_mappings(public_mappings, private_mappings)
    else:
        print("error: unsupported language: %s" % lang)
        return 1

    return 0


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--lang", choices=["c++", "imp"], default="c++",
                        help="output language")
    parser.add_argument("--verbose", "-v", action="store_true",
                        help="verbose output")
    parser.add_argument("rootdirs",
                        nargs="+",
                        help=("include roots (usually /usr/include/c++/11 "
                              "/usr/include/x86_64-linux-gnu/c++/11/)"))
    args = parser.parse_args()
    sys.exit(main(args.rootdirs, args.lang, args.verbose))
