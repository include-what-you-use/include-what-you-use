#!/usr/bin/env python

##===--- iwyu_tool_test.py - test for fix_includes.py ---------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

import unittest
import iwyu_tool

try:
    from cStringIO import StringIO
except ImportError:
    from io import StringIO

class MockInvocation(iwyu_tool.Invocation):
    def __init__(self, *args):
        iwyu_tool.Invocation.__init__(self, *args)
        self._will_return = ''
        self._will_block = 0

    def willBlock(self, seconds):
        self._will_block = seconds

    def willReturn(self, content):
        self._will_return = content

    def run_iwyu(self, verbose):
        if self._will_block > 0:
            from time import sleep
            sleep(self._will_block)
        return self._will_return


class IWYUToolTestBase(unittest.TestCase):
    def setUp(self):
        self.iwyu_args = []
        self.stdout_stub = StringIO()
        iwyu_tool.sys.stdout = self.stdout_stub

    def _getInvocation(self, entry):
        return MockInvocation.from_compile_command(entry, self.iwyu_args)

    def _processInvocations(self,
                            invocations,
                            verbose=False,
                            formatter=None,
                            jobs=1):
        if formatter is None:
            formatter = iwyu_tool.DEFAULT_FORMAT
        formatter = iwyu_tool.FORMATTERS.get(formatter, formatter)

        return iwyu_tool.process_invocations(invocations, verbose, formatter,
                                             jobs)


class IWYUToolTests(IWYUToolTestBase):
    def test_from_compile_command(self):
        self.iwyu_args = ['-foo']
        invocation = self._getInvocation({
            'directory':
            '/home/user/llvm/build',
            'command':
            '/usr/bin/clang++ -Iinclude file.cc',
            'file':
            'file.cc'
        })
        self.assertEqual(
            invocation.command,
            [iwyu_tool.IWYU_EXECUTABLE, '-foo', '-Iinclude', 'file.cc'])
        self.assertEqual(invocation.cwd, '/home/user/llvm/build')

    def test_invocation(self):
        invocation = MockInvocation([], '')
        invocation.willReturn('BAR')
        self.assertFalse(self._processInvocations([invocation]))
        self.assertEqual(self.stdout_stub.getvalue(), 'BAR\n')

    def test_order_asynchronous(self):
        invocations = [MockInvocation([], '') for _ in range(100)]
        for n, invocation in enumerate(invocations):
            invocation.willReturn('BAR%d' % n)
            from random import random
            invocation.willBlock(random() / 100)
        self.assertFalse(self._processInvocations(invocations, jobs=100))
        self.assertSetEqual(
            set('BAR%d' % n for n in range(100)),
            set(self.stdout_stub.getvalue()[:-1].split('\n')))

    def test_order_synchronous(self):
        invocations = [MockInvocation([], '') for _ in range(100)]
        for n, invocation in enumerate(invocations):
            invocation.willReturn('BAR%d' % n)
            from random import random
            invocation.willBlock(random() / 100)
        self.assertFalse(self._processInvocations(invocations, jobs=1))
        self.assertEqual(['BAR%d' % n for n in range(100)],
                         self.stdout_stub.getvalue()[:-1].split('\n'))


if __name__ == '__main__':
    unittest.main()
