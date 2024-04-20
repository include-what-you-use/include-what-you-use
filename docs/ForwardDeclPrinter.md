# RFC: Forward-declare mode for DeclPrinter

@vgvassilev and myself noticed by happenstance that we have similar needs in
Cling and IWYU: we want to be able to print valid forward-declarations from a
`NamedDecl`.

ROOT's Cling project has a fairly principled
[`ForwardDeclPrinter`](https://github.com/root-project/root/blob/master/interpreter/cling/lib/Interpreter/ForwardDeclPrinter.cpp).

The IWYU project has a very hacky but battle-tested [set
of](https://github.com/include-what-you-use/include-what-you-use/blob/93d8793081e5aa457114ef99d1e887390b370e4a/iwyu_output.cc#L394)
[functions](https://github.com/include-what-you-use/include-what-you-use/blob/93d8793081e5aa457114ef99d1e887390b370e4a/iwyu_output.cc#L469)
to produce exactly the kind of forward-declarations we need to paste into user
code.

It seems to us these needs could be met directly by `DeclPrinter`, with a custom
printing policy.

@vgvassilev has graciously offered to upstream relevant parts of the
`ForwardDeclPrinter` to teach `DeclPrinter` to do the work, and I'd like to
build out the `DeclPrinter` unittest suite to cover the body of examples we
already have in the IWYU test suite.

The obvious drawback is that there's no use inside the Clang source tree
itself, but it does seem like generally useful functionality.

We would love to hear what people think before we start working on patches.
