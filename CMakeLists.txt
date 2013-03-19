project(include-what-you-use)

add_clang_executable(include-what-you-use
  iwyu.cc
  iwyu_ast_util.cc
  iwyu_cache.cc
  iwyu_driver.cc
  iwyu_getopt.cc
  iwyu_globals.cc
  iwyu_include_picker.cc
  iwyu_lexer_utils.cc
  iwyu_location_util.cc
  iwyu_output.cc
  iwyu_path_util.cc
  iwyu_preprocessor.cc
  iwyu_verrs.cc
)

target_link_libraries(include-what-you-use
  clangFrontend
  clangSerialization
  clangDriver
  clangParse
  clangSema
  clangAnalysis
  clangAST
  clangLex
  clangBasic
  clangEdit
)

if( WIN32 )
  target_link_libraries(include-what-you-use
    shlwapi
  )
endif()

install(TARGETS include-what-you-use
  RUNTIME DESTINATION bin)
