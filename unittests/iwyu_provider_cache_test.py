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
import unittest


@unittest.skipUnless(sys.platform.startswith('linux'),
                     'Requires Linux address-space limits')
class ProviderCacheTest(unittest.TestCase):
    def test_shared_alias_components(self):
        import resource

        def limit_resources():
            soft, hard = resource.getrlimit(resource.RLIMIT_AS)
            cap = 1024 * 1024 * 1024
            if soft != resource.RLIM_INFINITY:
                cap = min(cap, soft)
            resource.setrlimit(resource.RLIMIT_AS, (cap, hard))
            resource.setrlimit(resource.RLIMIT_CORE, (0, 0))

        # Empty Pair bodies avoid a nested data-member traversal, so this
        # direct alias targets provider recursion. Twelve levels distinguish
        # repeated provider computation while keeping other helper costs small.
        source = ('template<class A, class B> struct Pair {};\n'
                  'template<class T> using Duplicate = Pair<T, T>;\n'
                  'using Result = ' + 'Duplicate<' * 12 + 'int' + '>' * 12 + ';\n'
                  'Result instance;\n')
        # Kill the owned child on timeout; allocation failure or timeout must
        # fail the test rather than exhausting the runner or stalling its suite.
        result = subprocess.run(
            [IWYU, '-std=c++11', '-x', 'c++', '-fsyntax-only', '-'],
            input=source, text=True, capture_output=True, timeout=15,
            preexec_fn=limit_resources)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertNotIn('should add these lines:', result.stderr)
        self.assertIn('has correct #includes/fwd-decls', result.stderr)


if __name__ == '__main__':
    IWYU = sys.argv.pop(1)
    unittest.main()
