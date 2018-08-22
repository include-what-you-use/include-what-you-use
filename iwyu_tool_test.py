#!/usr/bin/env python

##===-------------- iwyu_tool_test.py - test for iwyu_tool.py -------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##
import sys
import time
import random
import unittest
import iwyu_tool

try:
    from cStringIO import StringIO
except ImportError:
    from io import StringIO


class MockProcess(object):
    def __init__(self, block, content):
        self.block = block
        self.content = content
        self.complete_ts = time.time() + block

    def poll(self):
        if time.time() < self.complete_ts:
            return None
        return 0

    def get_output(self):
        self.poll()
        return self.content


class MockInvocation(iwyu_tool.Invocation):
    def __init__(self, command=None, cwd=''):
        iwyu_tool.Invocation.__init__(self, command or [], cwd)
        self._will_return = ''
        self._will_block = 0

    def will_block(self, seconds):
        self._will_block = seconds

    def will_return(self, content):
        self._will_return = content

    def start(self, verbose):
        if self._will_block > 0:
            time.sleep(self._will_block)
        return MockProcess(self._will_block, self._will_return)


class IWYUToolTests(unittest.TestCase):
    def _execute(self, invocations, verbose=False, formatter=None, jobs=1):
        formatter = formatter or iwyu_tool.DEFAULT_FORMAT
        formatter = iwyu_tool.FORMATTERS.get(formatter, formatter)
        return iwyu_tool.execute(invocations, verbose, formatter, jobs)

    def setUp(self):
        self.stdout_stub = StringIO()
        iwyu_tool.sys.stdout = self.stdout_stub

    def test_from_compile_command(self):
        iwyu_args = ['-foo']
        invocation = iwyu_tool.Invocation.from_compile_command(
            {
                'directory': '/home/user/llvm/build',
                'command': '/usr/bin/clang++ -Iinclude file.cc',
                'file': 'file.cc'
            }, iwyu_args)
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


if __name__ == '__main__':
    unittest.main()
