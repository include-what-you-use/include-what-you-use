# The Clang libraries are installed at the same place as the LLVM libraries.
get_filename_component(_IMPORT_PREFIX "${LLVM_LIBRARY_DIR}" PATH)

message(STATUS "Synthesized import prefix for Clang: ${_IMPORT_PREFIX}")

# Add library targets
add_library(clangBasic STATIC IMPORTED)
set_target_properties(clangBasic PROPERTIES
  INTERFACE_LINK_LIBRARIES "LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangBasic.a"
)

add_library(clangLex STATIC IMPORTED)
set_target_properties(clangLex PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangBasic;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangLex.a"
)

add_library(clangParse STATIC IMPORTED)
set_target_properties(clangParse PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangAST;clangBasic;clangLex;clangSema;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangParse.a"
)

add_library(clangAST STATIC IMPORTED)
set_target_properties(clangAST PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangBasic;clangLex;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangAST.a"
)

add_library(clangDynamicASTMatchers STATIC IMPORTED)
set_target_properties(clangDynamicASTMatchers PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangAST;clangASTMatchers;clangBasic;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangDynamicASTMatchers.a"
)

add_library(clangASTMatchers STATIC IMPORTED)
set_target_properties(clangASTMatchers PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangAST;clangBasic;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangASTMatchers.a"
)

add_library(clangCrossTU STATIC IMPORTED)
set_target_properties(clangCrossTU PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangAST;clangBasic;clangFrontend;clangIndex;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangCrossTU.a"
)

add_library(clangSema STATIC IMPORTED)
set_target_properties(clangSema PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangAST;clangAnalysis;clangBasic;clangEdit;clangLex;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangSema.a"
)

add_library(clangCodeGen STATIC IMPORTED)
set_target_properties(clangCodeGen PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangAnalysis;clangAST;clangAnalysis;clangBasic;clangFrontend;clangLex;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangCodeGen.a"
)

add_library(clangAnalysis STATIC IMPORTED)
set_target_properties(clangAnalysis PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangAST;clangBasic;clangLex;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangAnalysis.a"
)

add_library(clangEdit STATIC IMPORTED)
set_target_properties(clangEdit PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangAST;clangBasic;clangLex;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangEdit.a"
)

add_library(clangRewrite STATIC IMPORTED)
set_target_properties(clangRewrite PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangBasic;clangLex;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangRewrite.a"
)

add_library(clangARCMigrate STATIC IMPORTED)
set_target_properties(clangARCMigrate PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangAST;clangAnalysis;clangBasic;clangEdit;clangFrontend;clangLex;clangRewrite;clangSema;clangSerialization;clangStaticAnalyzerCheckers;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangARCMigrate.a"
)

add_library(clangDriver STATIC IMPORTED)
set_target_properties(clangDriver PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangBasic;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangDriver.a"
)

add_library(clangSerialization STATIC IMPORTED)
set_target_properties(clangSerialization PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangAST;clangBasic;clangLex;clangSema;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangSerialization.a"
)

add_library(clangRewriteFrontend STATIC IMPORTED)
set_target_properties(clangRewriteFrontend PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangAST;clangBasic;clangEdit;clangFrontend;clangLex;clangRewrite;clangSerialization;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangRewriteFrontend.a"
)

add_library(clangFrontend STATIC IMPORTED)
set_target_properties(clangFrontend PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangAST;clangBasic;clangDriver;clangEdit;clangLex;clangParse;clangSema;clangSerialization;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangFrontend.a"
)

add_library(clangFrontendTool STATIC IMPORTED)
set_target_properties(clangFrontendTool PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangBasic;clangCodeGen;clangDriver;clangFrontend;clangRewriteFrontend;clangARCMigrate;clangStaticAnalyzerFrontend;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangFrontendTool.a"
)

add_library(clangToolingCore STATIC IMPORTED)
set_target_properties(clangToolingCore PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangAST;clangBasic;clangLex;clangRewrite;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangToolingCore.a"
)

add_library(clangToolingRefactor STATIC IMPORTED)
set_target_properties(clangToolingRefactor PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangAST;clangASTMatchers;clangBasic;clangFormat;clangIndex;clangLex;clangRewrite;clangToolingCore;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangToolingRefactor.a"
)

add_library(clangToolingASTDiff STATIC IMPORTED)
set_target_properties(clangToolingASTDiff PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangBasic;clangAST;clangLex;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangToolingASTDiff.a"
)

add_library(clangTooling STATIC IMPORTED)
set_target_properties(clangTooling PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangAST;clangASTMatchers;clangBasic;clangDriver;clangFormat;clangFrontend;clangLex;clangRewrite;clangToolingCore;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangTooling.a"
)

add_library(clangIndex STATIC IMPORTED)
set_target_properties(clangIndex PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangAST;clangBasic;clangFormat;clangFrontend;clangRewrite;clangSerialization;clangToolingCore;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangIndex.a"
)

add_library(clangStaticAnalyzerCore STATIC IMPORTED)
set_target_properties(clangStaticAnalyzerCore PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangAST;clangASTMatchers;clangAnalysis;clangBasic;clangLex;clangRewrite;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangStaticAnalyzerCore.a"
)

add_library(clangStaticAnalyzerCheckers STATIC IMPORTED)
set_target_properties(clangStaticAnalyzerCheckers PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangAST;clangASTMatchers;clangAnalysis;clangBasic;clangLex;clangStaticAnalyzerCore;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangStaticAnalyzerCheckers.a"
)

add_library(clangStaticAnalyzerFrontend STATIC IMPORTED)
set_target_properties(clangStaticAnalyzerFrontend PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangAST;clangAnalysis;clangBasic;clangFrontend;clangLex;clangStaticAnalyzerCheckers;clangStaticAnalyzerCore;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangStaticAnalyzerFrontend.a"
)

add_library(clangFormat STATIC IMPORTED)
set_target_properties(clangFormat PROPERTIES
  INTERFACE_LINK_LIBRARIES "clangBasic;clangLex;clangToolingCore;LLVM"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libclangFormat.a"
)

# Cleanup temporary variables.
set(_IMPORT_PREFIX)
