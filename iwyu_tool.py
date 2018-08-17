#!/usr/bin/env python

from __future__ import print_function

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
import re
import sys
import json
import shlex
import argparse
import subprocess
import multiprocessing


CORRECT_RE = re.compile(r'^\((.*?) has correct #includes/fwd-decls\)$')
SHOULD_ADD_RE = re.compile(r'^(.*?) should add these lines:$')
SHOULD_REMOVE_RE = re.compile(r'^(.*?) should remove these lines:$')
FULL_LIST_RE = re.compile(r'The full include-list for (.*?):$')
END_RE = re.compile(r'^---$')
LINES_RE = re.compile(r'^- (.*?)  // lines ([0-9]+)-[0-9]+$')


GENERAL, ADD, REMOVE, LIST = range(4)


def clang_formatter(output):
    """ Process iwyu's output into something clang-like. """
    formatted = []

    state = (GENERAL, None)
    for line in output.splitlines():
        match = CORRECT_RE.match(line)
        if match:
            formatted.append('%s:1:1: note: #includes/fwd-decls are correct' %
                             match.groups(1))
            continue
        match = SHOULD_ADD_RE.match(line)
        if match:
            state = (ADD, match.group(1))
            continue
        match = SHOULD_REMOVE_RE.match(line)
        if match:
            state = (REMOVE, match.group(1))
            continue
        match = FULL_LIST_RE.match(line)
        if match:
            state = (LIST, match.group(1))
        elif END_RE.match(line):
            state = (GENERAL, None)
        elif not line.strip():
            continue
        elif state[0] == GENERAL:
            formatted.append(line)
        elif state[0] == ADD:
            formatted.append('%s:1:1: error: add the following line' % state[1])
            formatted.append(line)
        elif state[0] == REMOVE:
            match = LINES_RE.match(line)
            line_no = match.group(2) if match else '1'
            formatted.append('%s:%s:1: error: remove the following line' %
                             (state[1], line_no))
            formatted.append(match.group(1))

    return os.linesep.join(formatted)


DEFAULT_FORMAT = 'iwyu'
FORMATTERS = {
    'iwyu': lambda output: output,
    'clang': clang_formatter
}


IWYU_EXECUTABLE = 'include-what-you-use'
if sys.platform.startswith('win'):
    IWYU_EXECUTABLE += '.exe'


def run_iwyu(dbentry, extra_args, verbose):
    """ Transform dbentry to an IWYU command, and run it. """
    cwd = dbentry['directory']

    if 'arguments' in dbentry:
        # arguments is a command-line in list form.
        command = dbentry['arguments']
    elif 'command' in dbentry:
        # command is a command-line in string form, split to list.
        command = shlex.split(dbentry['command'])
    else:
        raise ValueError('Invalid compilation database entry: %s' % dbentry)

    # Rewrite the compile command for IWYU
    compile_command, compile_args = command[0], command[1:]
    if compile_command.endswith('cl.exe'):
        # If the compiler name is cl.exe, let IWYU be cl-compatible.
        extra_args = ['--driver-mode=cl'] + extra_args

    command = [IWYU_EXECUTABLE] + extra_args + compile_args

    if verbose:
        print('# ' + ' '.join(command))

    # Environment dictionary handling from https://stackoverflow.com/a/4453495
    # ensures that: The current environment is not altered, instead it is
    # copied for the child process, then the directory of iwyu_tool.py is
    # prepended to the PATH variable.
    env = os.environ.copy()
    path_variable = os.path.dirname(__file__)
    if 'PATH' in env:
        path_variable += os.pathsep + env['PATH']
    env['PATH'] = path_variable
    process = subprocess.Popen(command,
                               cwd=cwd,
                               env=env,
                               stdout=subprocess.PIPE,
                               stderr=subprocess.STDOUT)
    return process.communicate()[0].decode('utf-8')


def main(compilation_db_path, source_files, verbose, formatter, jobs, iwyu_args):
    """ Entry point. """
    # Canonicalize compilation database path
    if os.path.isdir(compilation_db_path):
        compilation_db_path = os.path.join(compilation_db_path,
                                           'compile_commands.json')

    compilation_db_path = os.path.realpath(compilation_db_path)
    if not os.path.isfile(compilation_db_path):
        print('ERROR: No such file or directory: \'%s\'' % compilation_db_path)
        return 1

    # Read compilation db from disk
    with open(compilation_db_path, 'r') as fileobj:
        compilation_db = json.load(fileobj)

    # expand symlinks
    for entry in compilation_db:
        entry['file'] = os.path.realpath(entry['file'])

    # Cross-reference source files with compilation database
    source_files = [os.path.realpath(s) for s in source_files]
    if not source_files:
        # No source files specified, analyze entire compilation database
        entries = compilation_db
    else:
        # Source files specified, analyze the ones appearing in compilation db,
        # warn for the rest.
        entries = []
        for source in source_files:
            if os.path.isdir(source) or os.path.isfile(source):
                entries.extend([e for e in compilation_db if e['file'].startswith(source)])
            else:
                print('WARNING: \'%s\' not found in compilation database.' %
                      source)

    try:
        pool = multiprocessing.Pool(jobs)
        # No actual results in `results`, it's only used for exception handling.
        # Details here: https://stackoverflow.com/a/28660669.
        results = []
        for entry in entries:
            results.append(pool.apply_async(run_iwyu,
                                            (entry, iwyu_args, verbose),
                                            callback=lambda x: print(formatter(x))))
        pool.close()
        pool.join()
        for r in results:
            r.get()
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

    # Parse arguments.
    parser = argparse.ArgumentParser(
        description='Include-what-you-use compilation database driver.',
        epilog='Assumes include-what-you-use is available on the PATH.')
    customize_usage(parser)
    customize_help(parser)

    parser.add_argument('-v', '--verbose', action='store_true',
                        help='Print IWYU commands')
    parser.add_argument('-o', '--output-format', type=str,
                        choices=FORMATTERS.keys(), default=DEFAULT_FORMAT,
                        help='Output format (default: %s)' % DEFAULT_FORMAT)
    parser.add_argument('-j', '--jobs', type=int, default=1,
                        help='Number of concurrent subprocesses')
    parser.add_argument('-p', metavar='<build-path>', required=True,
                        help='Compilation database path', dest='dbpath')
    parser.add_argument('source', nargs='*',
                        help=('Zero or more source files (or directories) to '
                              'run IWYU on. Defaults to all in compilation '
                              'database.'))

    def partition_args(argv):
        """ Split around '--' into driver args and IWYU args. """
        try:
            double_dash = argv.index('--')
            return argv[:double_dash], argv[double_dash+1:]
        except ValueError:
            return argv, []
    argv, iwyu_args = partition_args(sys.argv[1:])
    args = parser.parse_args(argv)

    # Force -Xiwyu prefix to iwyu_args so users don't have to provide prefix
    # explicitly.
    prefixes = ['-Xiwyu'] * len(iwyu_args)
    iwyu_args = list(sum(zip(prefixes, iwyu_args), ()))

    sys.exit(main(args.dbpath, args.source, args.verbose,
                  FORMATTERS[args.output_format], args.jobs, iwyu_args))


if __name__ == '__main__':
    _bootstrap()
