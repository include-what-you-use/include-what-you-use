##===- tools/include-what-you-use/Makefile -----------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

# If you get compile errors while building this, it may be that
# top-of-tree clang has changed its internal API a bit since the last
# iwyu commit.  One way to solve this is to use 'svn log' to figure
# out when the last commit to the include-what-you-use tree was (or
# visit http://code.google.com/p/include-what-you-use/source/list),
# and then sync clang and llvm to the revision they had at that time.

CLANG_LEVEL := ../..

TOOLNAME = include-what-you-use
NO_INSTALL = 1

# No plugins, optimize startup time.
TOOL_NO_EXPORTS = 1

LINK_COMPONENTS = ipo
USEDLIBS = clangFrontend.a clangSerialization.a clangDriver.a clangSema.a \
           clangAnalysis.a clangAST.a clangParse.a clangLex.a clangBasic.a

include $(CLANG_LEVEL)/Makefile
