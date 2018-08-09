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
import time
import shlex
import argparse
import tempfile
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


def find_include_what_you_use():
    """ Find IWYU executable and return its full pathname. """

    # TODO: Investigate using shutil.which when Python 2 has passed away.
    executable_name = 'include-what-you-use'
    if sys.platform.startswith('win'):
        executable_name += '.exe'

    search_path = [os.path.dirname(__file__)]
    search_path += os.environ.get('PATH', '').split(os.pathsep)

    for dirpath in search_path:
        full = os.path.join(dirpath, executable_name)
        if os.path.isfile(full):
            return full

    return None


IWYU_EXECUTABLE = find_include_what_you_use()

class Process(object):
    def __init__(self, proc, outfile):
        self.proc = proc
        self.outfile = outfile
        self.output = None

    def poll(self):
        return self.proc.poll()

    def get_output(self):
        if not self.output:
            self.proc.wait()
            self.outfile.seek(0)
            self.output = self.outfile.read().decode("utf-8")
            self.outfile.close()

        return self.output

    @classmethod
    def start(cls, invocation, verbose):
        if verbose:
            print('# %s' % invocation)

        outfile = tempfile.TemporaryFile(prefix='iwyu')
        process = subprocess.Popen(
            invocation.command,
            cwd=invocation.cwd,
            stdout=outfile,
            stderr=subprocess.STDOUT)
        return cls(process, outfile)

class Invocation(object):
    """ Holds arguments of an IWYU invocation. """
    def __init__(self, command, cwd):
        self.command = command
        self.cwd = cwd

    def __str__(self):
        return ' '.join(self.command)

    @classmethod
    def from_compile_command(cls, entry, extra_args):
        """ Parse a JSON compilation database entry into new Invocation. """
        if 'arguments' in entry:
            # arguments is a command-line in list form.
            command = entry['arguments']
        elif 'command' in entry:
            # command is a command-line in string form, split to list.
            command = shlex.split(entry['command'])
        else:
            raise ValueError('Invalid compilation database entry: %s' % entry)

        # Rewrite the compile command for IWYU
        compile_command, compile_args = command[0], command[1:]
        if compile_command.endswith('cl.exe'):
            # If the compiler name is cl.exe, let IWYU be cl-compatible.
            extra_args = ['--driver-mode=cl'] + extra_args

        command = [IWYU_EXECUTABLE] + extra_args + compile_args
        return cls(command, entry['directory'])


    def run(self, verbose):
        """ Run invocation and collect output. """
        if verbose:
            print('# %s' % self)

        return Process.start(self, verbose)


def parse_compilation_db(compilation_db_path):
    """ Parse JSON compilation database and return a canonicalized form. """
    if os.path.isdir(compilation_db_path):
        compilation_db_path = os.path.join(compilation_db_path,
                                           'compile_commands.json')

    # Read compilation db from disk.
    compilation_db_path = os.path.realpath(compilation_db_path)
    with open(compilation_db_path, 'r') as fileobj:
        compilation_db = json.load(fileobj)

    # Expand symlinks.
    for entry in compilation_db:
        entry['file'] = os.path.realpath(entry['file'])

    return compilation_db


def slice_compilation_db(compilation_db, selection):
    """ Return a new compilation database reduced to the paths in selection. """
    if not selection:
        return compilation_db

    # Canonicalize selection paths to match compilation database.
    selection = [os.path.realpath(p) for p in selection]

    new_db = []
    for path in selection:
        if not os.path.exists(path):
            print('WARNING: \'%s\' not found on disk.' % path)
            continue

        matches = [e for e in compilation_db if e['file'].startswith(path)]
        if not matches:
            print('WARNING: \'%s\' not found in compilation database.' % path)
            continue

        new_db.extend(matches)

    return new_db

def execute(invocations, verbose, formatter, jobs):
    if jobs == 1:
        for invocation in invocations:
            print(formatter(invocation.run(verbose).get_output()))
        return

    pending = []
    while invocations or pending:
        # get complete iwyu processes
        complete = [proc for proc in pending if proc.poll() is not None]

        for proc in complete:
            # handle iwyu output
            pending.remove(proc)
            print(formatter(proc.get_output()))
            proc.close()

        n = jobs - len(pending)
        pending.extend(e.run(verbose) for e in invocations[:n])
        invocations = invocations[n:]

        time.sleep(0) # yield cpu


def main(compilation_db_path, source_files, verbose, formatter, jobs, iwyu_args):
    """ Entry point. """

    try:
        compilation_db = parse_compilation_db(compilation_db_path)
    except IOError as why:
        print('Failed to parse JSON compilation database: %s' % why)
        return 1

    compilation_db = slice_compilation_db(compilation_db, source_files)

    # Transform compilation db entries into a list of IWYU invocations.
    invocations = [
        Invocation.from_compile_command(e, iwyu_args) for e in compilation_db
    ]

    return execute(invocations, verbose, formatter, jobs)

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
