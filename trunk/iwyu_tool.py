#!/usr/bin/env python

""" Driver to consume a Clang compilation database and invoke IWYU.

Example usage with CMake:

  # Unix systems
  $ mkdir build && cd build
  $ CC="clang" CXX="clang++" cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ...
  $ iwyu_tool.py -p .

  # Windows systems
  $ mkdir build && cd build
  $ cmake -DCMAKE_CXX_COMPILER="%VCINSTALLDIR%/bin/cl.exe" \
    -DCMAKE_C_COMPILER="%VCINSTALLDIR%/VC/bin/cl.exe" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -G Ninja ...
  $ python iwyu_tool.py -p .

See iwyu_tool.py -h for more details on command-line arguments.
"""

import os
import sys
import json
import argparse
import subprocess


def run_iwyu(cwd, compile_command, iwyu_args, verbose):
    """ Rewrite compile_command to an IWYU command, and run it. """
    compiler, _, args = compile_command.partition(' ')
    if compiler.endswith('cl.exe'):
        # If the compiler name is cl.exe, let IWYU be cl-compatible
        clang_args = ['--driver-mode=cl']
    else:
        clang_args = []

    iwyu_args = ['-Xiwyu ' + a for a in iwyu_args]
    command = ['include-what-you-use'] + clang_args + iwyu_args
    command = '%s %s' % (' '.join(command), args.strip())

    if verbose:
        print('%s:' % command)

    return subprocess.call(command, cwd=cwd, shell=True)


def main(compilation_db_path, source_files, verbose, iwyu_args):
    """ Entry point. """
    # Canonicalize compilation database path
    if os.path.isdir(compilation_db_path):
        compilation_db_path = os.path.join(compilation_db_path,
                                           'compile_commands.json')

    compilation_db_path = os.path.abspath(compilation_db_path)
    if not os.path.isfile(compilation_db_path):
        print('ERROR: No such file or directory: \'%s\'' % compilation_db_path)
        return 1

    # Read compilation db from disk
    with open(compilation_db_path, 'r') as fileobj:
        compilation_db = json.load(fileobj)

    # Cross-reference source files with compilation database
    source_files = [os.path.abspath(s) for s in source_files]
    if not source_files:
        # No source files specified, analyze entire compilation database
        entries = compilation_db
    else:
        # Source files specified, analyze the ones appearing in compilation db,
        # warn for the rest.
        entries = []
        for source in source_files:
            matches = [e for e in compilation_db if e['file'] == source]
            if matches:
                entries.extend(matches)
            else:
                print('WARNING: \'%s\' not found in compilation database.' %
                      source)

    # Run analysis
    try:
        for entry in entries:
            cwd, compile_command = entry['directory'], entry['command']
            run_iwyu(cwd, compile_command, iwyu_args, verbose)
    except OSError as why:
        print('ERROR: Failed to launch include-what-you-use: %s' % why)
        return 1

    return 0


def _bootstrap():
    """ Parse arguments and dispatch to main(). """
    # This hackery is necessary to add the forwarded IWYU args to the
    # usage and help strings.
    def customize_usage(parser):
        """ Rewrite the parser's format_usage. """
        original_format_usage = parser.format_usage
        parser.format_usage = lambda: original_format_usage().rstrip() + \
                              ' -- [<IWYU args>]' + os.linesep

    def customize_help(parser):
        """ Rewrite the parser's format_help. """
        original_format_help = parser.format_help

        def custom_help():
            """ Customized help string, calls the adjusted format_usage. """
            helpmsg = original_format_help()
            helplines = helpmsg.splitlines()
            helplines[0] = parser.format_usage().rstrip()
            return os.linesep.join(helplines) + os.linesep

        parser.format_help = custom_help

    # Parse arguments
    parser = argparse.ArgumentParser(
        description='Include-what-you-use compilation database driver.',
        epilog='Assumes include-what-you-use is available on the PATH.')
    customize_usage(parser)
    customize_help(parser)

    parser.add_argument('-v', '--verbose', action='store_true',
                        help='Print IWYU commands')
    parser.add_argument('-p', metavar='<build-path>', required=True,
                        help='Compilation database path', dest='dbpath')
    parser.add_argument('source', nargs='*',
                        help='Zero or more source files to run IWYU on. '
                        'Defaults to all in compilation database.')

    def partition_args(argv):
        """ Split around '--' into driver args and IWYU args. """
        try:
            dd = argv.index('--')
            return argv[:dd], argv[dd+1:]
        except ValueError:
            return argv, []

    argv, iwyu_args = partition_args(sys.argv[1:])
    args = parser.parse_args(argv)
    sys.exit(main(args.dbpath, args.source, args.verbose, iwyu_args))


if __name__ == '__main__':
    _bootstrap()
