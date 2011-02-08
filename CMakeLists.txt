project(include-what-you-use)

if( MSVC )
   add_definitions( /D_POSIX_=1 )
endif()

set(LLVM_USED_LIBS
  clangFrontend
  clangSerialization
  clangDriver
  clangSema
  clangAnalysis
  clangAST
  clangParse
  clangLex
  clangBasic
)

set(LLVM_LINK_COMPONENTS
  ipo
)

add_clang_executable(include-what-you-use
  iwyu.cc
  iwyu_cache.cc
  iwyu_globals.cc
  iwyu_include_picker.cc
  iwyu_lexer_utils.cc
  iwyu_location_util.cc
  iwyu_output.cc
  iwyu_preprocessor.cc
)
