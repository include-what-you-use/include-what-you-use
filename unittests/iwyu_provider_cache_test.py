#!/usr/bin/env python3

##===--- iwyu_provider_cache_test.py - bounded provider analysis test -----===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

"""Check recursive provider analysis without risking an unbounded CI run."""

import subprocess
import sys
import tempfile
import time
import unittest


def run_with_limits(command, source, timeout=15,
                    memory_limit=1024 * 1024 * 1024):
    import resource

    def disable_core_dumps():
        resource.setrlimit(resource.RLIMIT_CORE, (0, 0))

    # Sanitizers reserve large virtual shadow ranges. Watch resident memory
    # instead of restricting their address space. Files also keep diagnostic
    # output from consuming unbounded memory in the parent.
    with tempfile.TemporaryFile(mode='w+t') as stdout, \
            tempfile.TemporaryFile(mode='w+t') as stderr:
        with subprocess.Popen(command, stdin=subprocess.PIPE, stdout=stdout,
                              stderr=stderr, text=True,
                              preexec_fn=disable_core_dumps) as process:
            deadline = time.monotonic() + timeout
            try:
                process.stdin.write(source)
                process.stdin.close()
                while process.poll() is None:
                    if time.monotonic() >= deadline:
                        raise AssertionError(
                            'Provider analysis exceeded wall deadline')
                    try:
                        status_path = '/proc/{}/status'.format(process.pid)
                        with open(status_path) as status:
                            for line in status:
                                if line.startswith('VmRSS:'):
                                    resident_bytes = int(line.split()[1]) * 1024
                                    if resident_bytes > memory_limit:
                                        raise AssertionError(
                                            'Provider analysis exceeded '
                                            'resident-memory limit')
                                    break
                    except FileNotFoundError:
                        # The child can exit between poll() and opening status.
                        pass
                    time.sleep(0.01)
            finally:
                if process.poll() is None:
                    process.kill()
                process.wait()
            stdout.seek(0)
            stderr.seek(0)
            return subprocess.CompletedProcess(command, process.returncode,
                                               stdout.read(), stderr.read())


@unittest.skipUnless(sys.platform.startswith('linux'),
                     'Requires Linux resident-memory accounting')
class ResourceLimitTest(unittest.TestCase):
    def test_wall_deadline(self):
        command = [sys.executable, '-c', 'import time; time.sleep(60)']
        with self.assertRaisesRegex(AssertionError, 'wall deadline'):
            run_with_limits(command, '', timeout=0.05)

    def test_resident_memory_limit(self):
        code = 'import time; data = bytearray(64 * 1024 * 1024); time.sleep(60)'
        with self.assertRaisesRegex(AssertionError, 'resident-memory limit'):
            run_with_limits([sys.executable, '-c', code], '', timeout=5,
                            memory_limit=32 * 1024 * 1024)


@unittest.skipUnless(sys.platform.startswith('linux'),
                     'Requires Linux resident-memory accounting')
class ProviderCacheTest(unittest.TestCase):
    def test_shared_alias_components(self):
        # Empty Pair bodies avoid a nested data-member traversal, so this
        # direct alias targets provider recursion. Twelve levels distinguish
        # repeated provider computation while keeping other helper costs small.
        nested = 'Duplicate<' * 12 + 'int' + '>' * 12
        source = ('template<class A, class B> struct Pair {};\n'
                  'template<class T> using Duplicate = Pair<T, T>;\n'
                  'using Result = ' + nested + ';\n'
                  'Result instance;\n')
        result = run_with_limits(
            [IWYU, '-std=c++11', '-x', 'c++', '-fsyntax-only', '-'],
            source)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertNotIn('should add these lines:', result.stderr)
        self.assertIn('has correct #includes/fwd-decls', result.stderr)


if __name__ == '__main__':
    IWYU = sys.argv.pop(1)
    unittest.main()
