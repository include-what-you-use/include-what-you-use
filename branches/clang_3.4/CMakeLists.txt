cmake_minimum_required(VERSION 2.8)

project(include-what-you-use)

if( CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR )
  message(STATUS "IWYU out-of-tree configuration")

  if( NOT DEFINED LLVM_PATH )
    message(FATAL_ERROR "LLVM_PATH must be provided using -DLLVM_PATH=<path to llvm package root>")
  endif()

  link_directories(${LLVM_PATH}/lib)
  include_directories(${LLVM_PATH}/include)

  add_definitions(
    -D__STDC_LIMIT_MACROS
    -D__STDC_CONSTANT_MACROS
  )

  if( MSVC )
    # Adjust MSVC warning levels.
    add_definitions(
      # Ignore security warnings for standard functions.
      -D_CRT_SECURE_NO_WARNINGS
      -D_SCL_SECURE_NO_WARNINGS

      # Disabled warnings.
      -wd4146 # Suppress 'unary minus operator applied to unsigned type, result still unsigned'
      -wd4244 # Suppress ''argument' : conversion from 'type1' to 'type2', possible loss of data'
      -wd4291 # Suppress ''declaration' : no matching operator delete found; memory will not be freed if initialization throws an exception'
      -wd4345 # Suppress 'behavior change: an object of POD type constructed with an initializer of the form () will be default-initialized'
      -wd4355 # Suppress ''this' : used in base member initializer list'
      -wd4624 # Suppress ''derived class' : destructor could not be generated because a base class destructor is inaccessible'
      -wd4800 # Suppress ''type' : forcing value to bool 'true' or 'false' (performance warning)'

      # Promoted warnings.
      -w14062 # Promote 'enumerator in switch of enum is not handled' to level 1 warning.

      # Promoted warnings to errors.
      -we4238 # Promote 'nonstandard extension used : class rvalue used as lvalue' to error.
    )
  endif()
else()
  message(STATUS "IWYU in-tree configuration")
endif()

add_executable(include-what-you-use
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

if( MSVC )
  # Put project in solution folder
  set_target_properties(include-what-you-use
    PROPERTIES FOLDER "Clang executables"
  )
else()
  # Disable RTTI to be compatible with LLVM/Clang libraries.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

# Clang dependencies.
target_link_libraries(include-what-you-use
  clangFrontend
  clangSerialization
  clangDriver
  clangParse
  clangSema
  clangAnalysis
  clangAST
  clangBasic
  clangEdit
  clangLex
)

# LLVM dependencies.
target_link_libraries(include-what-you-use
  LLVMX86AsmParser # MC, MCParser, Support, X86Desc, X86Info
  LLVMX86Desc # MC, Support, X86AsmPrinter, X86Info
  LLVMX86AsmPrinter # MC, Support, X86Utils
  LLVMX86Info # MC, Support, Target
  LLVMX86Utils # Core, Support
  LLVMipo
  LLVMScalarOpts
  LLVMInstCombine
  LLVMTransformUtils
  LLVMipa
  LLVMAnalysis
  LLVMTarget
  LLVMMC # Object, Support
  LLVMMCParser # MC, Support
  LLVMObject # Support
  LLVMCore # Support
  LLVMSupport
  LLVMBitReader
  LLVMOption
)

# Platform dependencies.
if( WIN32 )
  target_link_libraries(include-what-you-use
    shlwapi
  )
else()
  target_link_libraries(include-what-you-use
    pthread
    dl
    curses
  )
endif()

install(TARGETS include-what-you-use
  RUNTIME DESTINATION bin)
