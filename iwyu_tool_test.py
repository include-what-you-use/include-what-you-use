#!/usr/bin/env python3

##===--- iwyu_tool_test.py - test for iwyu_tool.py ------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##
import os
import sys
import time
import random
import inspect
import unittest
import iwyu_tool

try:
    from cStringIO import StringIO
except ImportError:
    from io import StringIO


class MockProcess(object):
    def __init__(self, block, content, returncode):
        self.content = content
        self.complete_ts = time.time() + block
        self.returncode = returncode

    def poll(self):
        if time.time() < self.complete_ts:
            return None
        return 0

    def get_output(self):
        remaining = self.complete_ts - time.time()
        if remaining > 0:
            time.sleep(remaining)
        return self.content


class MockInvocation(iwyu_tool.Invocation):
    def __init__(self, command=None, cwd=''):
        iwyu_tool.Invocation.__init__(self, command or [], cwd)
        self._will_return = ''
        self._will_block = 0
        self._will_returncode = 0

    def will_block(self, seconds):
        self._will_block = seconds

    def will_return(self, content):
        self._will_return = content

    def will_returncode(self, returncode):
        self._will_returncode = returncode

    def start(self, verbose):
        return MockProcess(self._will_block, self._will_return,
                           self._will_returncode)


class MockIwyuToolMain(object):
    """ Replacement for iwyu_tool.main to capture parsed arguments. """
    def __init__(self):
        if hasattr(inspect, 'getfullargspec'):
            getargspec = inspect.getfullargspec
        else:
            getargspec = inspect.getargspec
        self.argspec = getargspec(iwyu_tool.main).args
        self.real_iwyu_tool_main = iwyu_tool.main
        iwyu_tool.main = self._mock
        self.call_args = {}

    def reset(self):
        iwyu_tool.main = self.real_iwyu_tool_main

    def _mock(self, *args, **kwargs):
        for i, arg in enumerate(args):
            name = self.argspec[i]
            self.call_args[name] = arg

        self.call_args.update(kwargs)
        return 0


class IWYUToolTests(unittest.TestCase):
    def _execute(self, invocations, verbose=False, formatter=None, jobs=1):
        formatter = formatter or iwyu_tool.DEFAULT_FORMAT
        formatter = iwyu_tool.FORMATTERS.get(formatter, formatter)
        return iwyu_tool.execute(invocations, verbose, formatter, jobs)

    def setUp(self):
        self.stdout_stub = StringIO()
        iwyu_tool.sys.stdout = self.stdout_stub

    def test_from_compile_command(self):
        extra_args = ['-foo']
        invocation = iwyu_tool.Invocation.from_compile_command(
            {
                'directory': '/home/user/llvm/build',
                'command': '/usr/bin/clang++ -Iinclude file.cc',
                'file': 'file.cc'
            }, extra_args)
        self.assertEqual(
            invocation.command,
            [iwyu_tool.IWYU_EXECUTABLE, '-foo', '-Iinclude', 'file.cc'])
        self.assertEqual(invocation.cwd, '/home/user/llvm/build')

    def test_invocation(self):
        invocation = MockInvocation()
        invocation.will_return('BAR')
        self._execute([invocation])
        self.assertEqual(self.stdout_stub.getvalue(), 'BAR\n')

    def test_order_asynchronous(self):
        invocations = [MockInvocation() for _ in range(100)]
        for n, invocation in enumerate(invocations):
            invocation.will_return('BAR%d' % n)
            invocation.will_block(random.random() / 100)
        self._execute(invocations, jobs=100)
        self.assertSetEqual(
            set('BAR%d' % n for n in range(100)),
            set(self.stdout_stub.getvalue().splitlines()))

    def test_order_synchronous(self):
        invocations = [MockInvocation() for _ in range(100)]
        for n, invocation in enumerate(invocations):
            invocation.will_return('BAR%d' % n)
            invocation.will_block(random.random() / 100)
        self._execute(invocations, jobs=1)
        self.assertEqual(['BAR%d' % n for n in range(100)],
                         self.stdout_stub.getvalue().splitlines())

    def test_returncode(self):
        invocation = MockInvocation()
        invocation.will_returncode(0)
        self.assertEqual(self._execute([invocation]), 0)
        invocation = MockInvocation()
        invocation.will_returncode(1)
        self.assertEqual(self._execute([invocation]), 1)
        invocation = MockInvocation()
        invocation.will_returncode(2)
        self.assertEqual(self._execute([invocation]), 2)

    def test_returncode_asynchronous(self):
        invocations = [MockInvocation() for _ in range(100)]
        for invocation in invocations:
            invocation.will_returncode(0)
            invocation.will_block(random.random() / 100)
        self.assertEqual(self._execute(invocations, jobs=100), 0)
        invocations = [MockInvocation() for _ in range(100)]
        for invocation in invocations:
            invocation.will_returncode(2)
            invocation.will_block(random.random() / 100)
        self.assertEqual(self._execute(invocations, jobs=100), 2)
        invocations = [MockInvocation() for _ in range(100)]
        for n, invocation in enumerate(invocations):
            invocation.will_returncode(6 if n == 0 else 2)
            invocation.will_block(random.random() / 100)
        self.assertEqual(self._execute(invocations, jobs=100), 6)

    def test_returncode_synchronous(self):
        invocations = [MockInvocation() for _ in range(1)]
        for invocation in invocations:
            invocation.will_returncode(0)
            invocation.will_block(random.random() / 100)
        self.assertEqual(self._execute(invocations, jobs=100), 0)
        invocations = [MockInvocation() for _ in range(1)]
        for invocation in invocations:
            invocation.will_returncode(2)
            invocation.will_block(random.random() / 100)
        self.assertEqual(self._execute(invocations, jobs=100), 2)
        invocations = [MockInvocation() for _ in range(1)]
        for n, invocation in enumerate(invocations):
            invocation.will_returncode(6 if n == 0 else 2)
            invocation.will_block(random.random() / 100)
        self.assertEqual(self._execute(invocations, jobs=100), 6)

    def test_worst_returncode(self):
        # If no invocations fail, we return zero
        invocations = [MockInvocation() for _ in range(4)]
        exit_codes = [0, 0, 0, 0]
        for invocation, exit_code in zip(invocations, exit_codes):
            invocation.will_returncode(exit_code)
        self.assertEqual(self._execute(invocations), 0)

        # If one invocation fails, its exit code is returned
        invocations = [MockInvocation() for _ in range(4)]
        exit_codes = [0, 1, 0, 0]
        for invocation, exit_code in zip(invocations, exit_codes):
            invocation.will_returncode(exit_code)
        self.assertEqual(self._execute(invocations), 1)

        # The highest of all positive exit codes is returned
        invocations = [MockInvocation() for _ in range(4)]
        exit_codes = [0, 1, 2, 3]
        for invocation, exit_code in zip(invocations, exit_codes):
            invocation.will_returncode(exit_code)
        self.assertEqual(self._execute(invocations), 3)

        # The lowest of all negative exit codes is returned
        invocations = [MockInvocation() for _ in range(4)]
        exit_codes = [-9, -10, -15, -1]
        for invocation, exit_code in zip(invocations, exit_codes):
            invocation.will_returncode(exit_code)
        self.assertEqual(self._execute(invocations), -15)

        # Negative take precedence over positive/zero
        invocations = [MockInvocation() for _ in range(4)]
        exit_codes = [0, 1, -1, 1]
        for invocation, exit_code in zip(invocations, exit_codes):
            invocation.will_returncode(exit_code)
        self.assertEqual(self._execute(invocations), -1)

    @unittest.skipIf(sys.platform.startswith('win'), "POSIX only")
    def test_is_subpath_of_posix(self):
        self.assertTrue(iwyu_tool.is_subpath_of('/a/b/c.c', '/a/b'))
        self.assertTrue(iwyu_tool.is_subpath_of('/a/b/c.c', '/a/b/'))
        self.assertTrue(iwyu_tool.is_subpath_of('/a/b/c.c', '/a/b/c.c'))
        self.assertFalse(iwyu_tool.is_subpath_of('/a/b/c.c', '/a/b/c'))
        self.assertFalse(iwyu_tool.is_subpath_of('/a/b/c.c', '/a/x'))
        # No case-insensitive match.
        self.assertFalse(iwyu_tool.is_subpath_of('/A/Bee/C.c', '/a/BEE'))

    @unittest.skipIf(not sys.platform.startswith('win'), "Windows only")
    def test_is_subpath_of_windows(self):
        self.assertTrue(iwyu_tool.is_subpath_of('\\a\\b\\c.c', '\\a\\b'))
        self.assertTrue(iwyu_tool.is_subpath_of('\\a\\b\\c.c', '\\a\\b\\'))
        self.assertTrue(iwyu_tool.is_subpath_of('\\a\\b\\c.c', '\\a\\b\\c.c'))
        self.assertFalse(iwyu_tool.is_subpath_of('\\a\\b\\c.c', '\\a\\b\\c'))
        self.assertFalse(iwyu_tool.is_subpath_of('\\a\\b\\c.c', '\\a\\x'))
        # Case-insensitive match.
        self.assertTrue(iwyu_tool.is_subpath_of('C:\\Bee\\C.c', 'c:\\BEE'))

    def test_from_cl_compile_command(self):
        invocation = iwyu_tool.Invocation.from_compile_command(
            {
                'directory': '/a',
                'command': 'cl.exe -I. x.cc',
                'file': 'x.cc'
            }, [])
        # Adds --driver-mode=cl if argv[0] is MSVC driver.
        self.assertEqual(
            invocation.command,
            [iwyu_tool.IWYU_EXECUTABLE, '--driver-mode=cl', '-I.', 'x.cc'])

    def test_is_msvc_driver(self):
        self.assertTrue(iwyu_tool.is_msvc_driver("cl.exe"))
        self.assertTrue(iwyu_tool.is_msvc_driver("clang-cl.exe"))
        self.assertTrue(iwyu_tool.is_msvc_driver("clang-cl"))
        self.assertFalse(iwyu_tool.is_msvc_driver("something"))

    @unittest.skipIf(not sys.platform.startswith('win'), 'Windows only')
    def test_is_msvc_driver_windows(self):
        # Case-insensitive match on Windows
        self.assertTrue(iwyu_tool.is_msvc_driver("CL.EXE"))
        self.assertTrue(iwyu_tool.is_msvc_driver("Clang-CL.exe"))
        self.assertTrue(iwyu_tool.is_msvc_driver("Clang-CL"))

    def test_split_command(self):
        self.assertEqual(['a', 'b', 'c d'],
                         iwyu_tool.split_command('a b "c d"'))

        self.assertEqual(['c', '-Idir/with spaces', 'x'],
                         iwyu_tool.split_command('c -I"dir/with spaces" x'))


class WinSplitTests(unittest.TestCase):
    """ iwyu_tool.win_split is subtle and complex enough that it warrants a
    dedicated test suite.
    """

    def assert_win_split(self, cmdstr, expected):
        self.assertEqual(expected,
                         iwyu_tool.win_split(cmdstr))

    def test_msdn_examples(self):
        """ Examples from below, detailing how to parse command-lines:
        https://msdn.microsoft.com/en-us/library/windows/desktop/17w5ykft.aspx
        """
        self.assert_win_split('"abc" d e',
                              ['abc', 'd', 'e'])
        self.assert_win_split(r'a\\b d"e f"g h',
                              [r'a\\b', 'de fg', 'h'])
        self.assert_win_split(r'a\\\"b c d',
                              [r'a\"b', 'c', 'd'])
        self.assert_win_split(r'a\\\\"b c" d e',
                              [r'a\\b c', 'd', 'e'])

        # Extra: odd number of backslashes before non-quote (should be
        # interpreted literally).
        self.assert_win_split(r'a\\\b d"e f"g h',
                              [r'a\\\b', 'de fg', 'h'])

    def test_trailing_backslash(self):
        """ Check that args with trailing backslash are retained. """
        self.assert_win_split('a\\ b c', ['a\\', 'b', 'c'])
        self.assert_win_split('a\\\\ b c', ['a\\\\', 'b', 'c'])

        # Last arg has dedicated handling, make sure backslashes are flushed.
        self.assert_win_split('b c a\\', ['b', 'c', 'a\\'])

    def test_cmake_examples(self):
        """ Example of observed CMake outputs that are hard to split. """
        self.assert_win_split(r'-I"..\tools\clang\tools\iwyu\inc ludes" -A',
                              [r'-I..\tools\clang\tools\iwyu\inc ludes', '-A'])

        self.assert_win_split(r'clang -Idir\\using\\os\\seps f.cc',
                              ['clang', r'-Idir\\using\\os\\seps', 'f.cc'])

        self.assert_win_split(r'clang -Idir\using\os\seps f.cc',
                              ['clang', r'-Idir\using\os\seps', 'f.cc'])

    def test_consecutive_spaces(self):
        """ Consecutive spaces outside of quotes should be folded. """
        self.assert_win_split('clang  -I.      -A',
                              ['clang', '-I.', '-A'])

        self.assert_win_split('clang  -I. \t     -A',
                              ['clang', '-I.', '-A'])


class BootstrapTests(unittest.TestCase):
    def setUp(self):
        self.main = MockIwyuToolMain()

    def tearDown(self):
        self.main.reset()

    def test_argparse_args(self):
        """ Argparse arguments are forwarded to main. """
        argv = ['iwyu_tool.py', '-v', '-o', 'clang', '-j', '12', '-p', '.',
                'src1', 'src2']
        iwyu_tool._bootstrap(argv)
        self.assertEqual('.', self.main.call_args['compilation_db_path'])
        self.assertEqual(['src1', 'src2'], self.main.call_args['source_files'])
        self.assertEqual(True, self.main.call_args['verbose'])
        self.assertEqual(iwyu_tool.FORMATTERS['clang'],
                         self.main.call_args['formatter'])
        self.assertEqual(12, self.main.call_args['jobs'])
        self.assertEqual([], self.main.call_args['extra_args'])

    def test_extra_args(self):
        """ Extra arguments after '--' are forwarded to main. """
        argv = ['iwyu_tool.py', '-p', '.', '--', '-extra1', '-extra2']
        iwyu_tool._bootstrap(argv)
        self.assertEqual(['-extra1', '-extra2'],
                         self.main.call_args['extra_args'])

    def test_extra_iwyu_args(self):
        """ Extra arguments with '-Xiwyu' prefix are forwarded verbatim. """
        argv = ['iwyu_tool.py', '-p', '.', '--', '-Xiwyu', '--arg']
        iwyu_tool._bootstrap(argv)
        self.assertEqual(['-Xiwyu', '--arg'], self.main.call_args['extra_args'])

    def test_extra_args_with_sep(self):
        """ If there are multiple '--' separators, subsequent ones are forwarded
        verbatim as part of extra arguments. """
        argv = ['iwyu_tool.py', '-p', '.', '--', 'arg1', '--', 'another_arg1']
        iwyu_tool._bootstrap(argv)
        self.assertEqual(['arg1', '--', 'another_arg1'],
                         self.main.call_args['extra_args'])

    def test_jobs_normal(self):
        """ Nonzero values for -j are forwarded as they are. """
        argv = ['iwyu_tool.py', '-p', '.', '-j', '4']
        iwyu_tool._bootstrap(argv)
        self.assertEqual(4, self.main.call_args['jobs'])


    def test_jobs_zero(self):
        """ -j=0 is translated to a nonzero job count. """
        argv = ['iwyu_tool.py', '-p', '.', '-j', '0']
        iwyu_tool._bootstrap(argv)
        self.assertIsNotNone(self.main.call_args['jobs'])
        self.assertLess(0, self.main.call_args['jobs'],
                        'The automatically chosen job count should be >0.')

    def test_jobs_novalue(self):
        """ It is allowed to use -j without a value. """
        argv = ['iwyu_tool.py', '-j', '-p', '.']
        iwyu_tool._bootstrap(argv)
        self.assertIsNotNone(self.main.call_args['jobs'])
        self.assertLess(0, self.main.call_args['jobs'],
                        'The automatically chosen job count should be >0.')

    def test_jobs_default(self):
        """ By default, we run singlethreaded. """
        argv = ['iwyu_tool.py', '-p', '.']
        iwyu_tool._bootstrap(argv)
        self.assertEqual(1, self.main.call_args['jobs'])


class CompilationDBTests(unittest.TestCase):
    def setUp(self):
        self.cwd = os.path.realpath(os.getcwd())

    def test_fixup_compilation_db(self):
        """ Compilation database path canonicalization. """
        compilation_db = [
            {
                "file": "Test.cpp"
            }
        ]

        canonical = iwyu_tool.fixup_compilation_db(compilation_db)

        # Check that file path is made absolute.
        entry = canonical[0]
        self.assertEqual(os.path.join(self.cwd, 'Test.cpp'), entry['file'])

    def test_fixup_from_entry_dir(self):
        """ Compilation database abs path is based on an entry's directory. """

        # Use a root dir from uuidgen so we don't risk hitting a real path.
        compilation_db = [
            {
                "directory": "/c057f113f69311e990bf54a05050d914/foobar",
                "file": "Test.cpp"
            }
        ]

        canonical = iwyu_tool.fixup_compilation_db(compilation_db)

        # Check that the file path is relative to the directory entry,
        # not to the current directory.
        entry = canonical[0]
        self.assertEqual('/c057f113f69311e990bf54a05050d914/foobar/Test.cpp',
                         entry['file'])

    def test_slice_compilation_db(self):
        file = os.path.realpath(__file__)

        compilation_db = [
            {
                "file": file,
            }
        ]

        new_db = iwyu_tool.slice_compilation_db(compilation_db, [file], [])
        self.assertEqual(new_db, compilation_db)

        new_db = iwyu_tool.slice_compilation_db(compilation_db, ["abc.cpp"], [])
        self.assertEqual(new_db, [])

        new_db = iwyu_tool.slice_compilation_db(compilation_db, [file], [file])
        self.assertEqual(new_db, [])

        new_db = iwyu_tool.slice_compilation_db(compilation_db, [], [])
        self.assertEqual(new_db, compilation_db)

        new_db = iwyu_tool.slice_compilation_db(compilation_db, [], [file])
        self.assertEqual(new_db, [])

    def test_unwrap_compile_command(self):
        """ Wrapping compile commands should be unwrapped. """
        compilation_db = {
            'directory': '/home/user/llvm/build',
            "command": "ccache cc -c test.c"
        }

        invocation = iwyu_tool.Invocation.from_compile_command(compilation_db, [])

        self.assertEqual(
            invocation.command,
            [iwyu_tool.IWYU_EXECUTABLE, '-c', 'test.c'])


class FindIWYUTests(unittest.TestCase):
    def test_iwyu_binary_made_absolute(self):
        oldval = os.environ.get('IWYU_BINARY')
        try:
            # Seed IWYU_BINARY with a relative path.
            os.environ['IWYU_BINARY'] = './build/include-what-you-use'

            # Check that find_iwyu returns absolute.
            path = iwyu_tool.find_include_what_you_use()
            self.assertTrue(os.path.isabs(path), 'Expected absolute: %r' % path)
        finally:
            if oldval:
                os.environ['IWYU_BINARY'] = oldval


if __name__ == '__main__':
    unittest.main()
