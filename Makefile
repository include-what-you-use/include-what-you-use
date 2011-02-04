##===- tools/include-what-you-use/Makefile -----------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

# If you get compile errors while building this, try syncing your
# version of LLVM+Clang to the revision include-what-you-use was
# developed against:
#LLVM_AND_CLANG_REVISION = r124350

CLANG_LEVEL := ../..

TOOLNAME = include-what-you-use
NO_INSTALL = 1

# No plugins, optimize startup time.
TOOL_NO_EXPORTS = 1

LINK_COMPONENTS = ipo
USEDLIBS = clangFrontend.a clangSerialization.a clangDriver.a clangSema.a \
           clangAnalysis.a clangAST.a clangParse.a clangLex.a clangBasic.a

include $(CLANG_LEVEL)/Makefile
