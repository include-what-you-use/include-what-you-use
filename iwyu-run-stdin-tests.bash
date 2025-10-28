#!/bin/bash

##===--- iwyu-run-stdin-tests.bash - run minimal stdin tests --------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

# This is a stupidly minimal test runner to execute include-what-you-use over a
# set of files input via stdin. It doesn't verify anything other than that
# include-what-you-use completes without error (e.g. due to CHECK_ failures).
# TODO: port this special run-mode into the real test runner.

set -o pipefail

if [ -n "$1" ]; then
    iwyu="$(realpath $1)"
else
    iwyu="$(which include-what-you-use)"
fi

run_iwyu() {
    local input="$1"
    echo ">>> Running $iwyu -I . -xc++ - < $input:"
    "$iwyu" -I . -xc++ - < "$input"
}

# Always run from root
cd "$(git rev-parse --show-toplevel)"

num_fail=0
num_pass=0
num_tests=0

for input in tests/stdin/*.cc; do
    if ! run_iwyu "$input" ; then
        echo "FAILED: $input"
        num_fail=$((num_fail+1))
    else
        num_pass=$((num_pass+1))
    fi
    num_tests=$((num_tests+1))
    echo
done

echo "$((num_tests)) tests executed, $num_pass passed, $num_fail failed"
exit $num_fail
