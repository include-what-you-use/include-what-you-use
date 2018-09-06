#!/usr/bin/env python

##===--- iwyu_tool_test.py - test for fix_includes.py ---------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

import time
import random
import unittest
import iwyu_tool

try:
    from cStringIO import StringIO
except ImportError:
    from io import StringIO

class MockInvocation(iwyu_tool.Invocation):
    def __init__(self, command=None, cwd=''):
        iwyu_tool.Invocation.__init__(self, command or [], cwd)
        self._will_return = ''
        self._will_block = 0

    def will_block(self, seconds):
        self._will_block = seconds

    def will_return(self, content):
        self._will_return = content

    def run_iwyu(self, verbose):
        if self._will_block > 0:
            time.sleep(self._will_block)
        return self._will_return


class IWYUToolTestBase(unittest.TestCase):
    def setUp(self):
        self.iwyu_args = []
        self.stdout_stub = StringIO()
        iwyu_tool.sys.stdout = self.stdout_stub

    def _process_invocations(self,
                             invocations,
                             verbose=False,
                             formatter=None,
                             jobs=1):
        formatter = formatter or iwyu_tool.DEFAULT_FORMAT
        formatter = iwyu_tool.FORMATTERS.get(formatter, formatter)

        return iwyu_tool.execute(invocations, verbose, formatter, jobs)


class IWYUToolTests(IWYUToolTestBase):
    def test_from_compile_command(self):
        self.iwyu_args = ['-foo']
        invocation = iwyu_tool.Invocation.from_compile_command(
            {
                'directory': '/home/user/llvm/build',
                'command': '/usr/bin/clang++ -Iinclude file.cc',
                'file': 'file.cc'
            }, self.iwyu_args)
        self.assertEqual(
            invocation.command,
            [iwyu_tool.IWYU_EXECUTABLE, '-foo', '-Iinclude', 'file.cc'])
        self.assertEqual(invocation.cwd, '/home/user/llvm/build')

    def test_invocation(self):
        invocation = MockInvocation([], '')
        invocation.will_return('BAR')
        self.assertEqual(self._process_invocations([invocation]), 0)
        self.assertEqual(self.stdout_stub.getvalue(), 'BAR\n')

    def test_order_asynchronous(self):
        invocations = [MockInvocation([], '') for _ in range(100)]
        for n, invocation in enumerate(invocations):
            invocation.will_return('BAR%d' % n)
            invocation.will_block(random.random() / 100)
        self.assertEqual(self._process_invocations(invocations, jobs=100), 0)
        self.assertSetEqual(
            set('BAR%d' % n for n in range(100)),
            set(self.stdout_stub.getvalue()[:-1].splitlines()))

    def test_order_synchronous(self):
        invocations = [MockInvocation([], '') for _ in range(100)]
        for n, invocation in enumerate(invocations):
            invocation.will_return('BAR%d' % n)
            invocation.will_block(random.random() / 100)
        self.assertEqual(self._process_invocations(invocations, jobs=1), 0)
        self.assertEqual(['BAR%d' % n for n in range(100)],
                         self.stdout_stub.getvalue()[:-1].splitlines())


if __name__ == '__main__':
    unittest.main()
