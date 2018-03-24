cmake_minimum_required(VERSION 3.4.3)

if( POLICY CMP0048 )
  # Silence CMP0048 warning about missing project VERSION.
  cmake_policy(SET CMP0048 NEW)
endif()

project(include-what-you-use)

if( CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR )
  message(STATUS "IWYU out-of-tree configuration")

  if( DEFINED LLVM_PATH )
    set(IWYU_LLVM_ROOT_PATH ${LLVM_PATH})
    message(WARNING "LLVM_PATH is deprecated, use IWYU_LLVM_ROOT_PATH instead.")
  endif()

  if( DEFINED IWYU_LLVM_INCLUDE_PATH AND DEFINED IWYU_LLVM_LIB_PATH )
    # User provided include/lib paths, fall through
  elseif ( DEFINED IWYU_LLVM_ROOT_PATH )
    # Synthesize include/lib relative to a root.
    set(IWYU_LLVM_INCLUDE_PATH ${IWYU_LLVM_ROOT_PATH}/include)
    set(IWYU_LLVM_LIB_PATH ${IWYU_LLVM_ROOT_PATH}/lib)
  else()
    # If none provided, fail.
    message(FATAL_ERROR
      "Don't know how to find LLVM headers/libraries. "
      "Use -DIWYU_LLVM_ROOT_PATH=/xyz or both "
      "-DIWYU_LLVM_INCLUDE_PATH=/xyz/include and -DIWYU_LLVM_LIB_PATH=/xyz/lib")
  endif()

  include_directories(${IWYU_LLVM_INCLUDE_PATH})
  link_directories(${IWYU_LLVM_LIB_PATH})

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
      -wd4141 # Suppress ''modifier' : used more than once' (because of __forceinline combined with inline)
      -wd4146 # Suppress 'unary minus operator applied to unsigned type, result still unsigned'
      -wd4244 # Suppress ''argument' : conversion from 'type1' to 'type2', possible loss of data'
      -wd4267 # Suppress ''var' : conversion from 'size_t' to 'type', possible loss of data'
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

  if( APPLE )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden")
  endif()
else()
  message(STATUS "IWYU in-tree configuration")
endif()

# Pick up Git revision so we can report it in version information.
include(FindGit)
if( GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git" )
  execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE IWYU_GIT_REV
    OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
  message(STATUS "Warning: IWYU Git version info not found, DO NOT release "
                 "from this build tree!")
endif()
add_definitions(-DIWYU_GIT_REV="${IWYU_GIT_REV}")

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

if( MINGW )
  # Work around 'too many sections' error with MINGW/GCC
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj")
endif()

if( MSVC )
  # Disable warnings for IWYU, and disable exceptions in MSVC's STL.
  add_definitions(
    -wd4722 # Suppress ''destructor'' : destructor never returns, potential memory leak
    -D_HAS_EXCEPTIONS=0
  )

  # Enable bigobj support and sane C++ exception semantics.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /EHsc")

  # Put project in solution folder
  set_target_properties(include-what-you-use
    PROPERTIES FOLDER "Clang executables"
  )
else()
  # Disable RTTI, use C++11 to be compatible with LLVM/Clang libraries.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -std=c++11")
endif()

# Clang dependencies.
target_link_libraries(include-what-you-use
  PRIVATE
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
  PRIVATE
  LLVMX86AsmParser # MC, MCParser, Support, X86CodeGen, X86Desc, X86Info
  LLVMX86CodeGen # Analysis, AsmPrinter, CodeGen, Core, MC, Support, Target, 
                 # X86AsmPrinter, X86Desc, X86Info, X86Utils
  LLVMX86Desc # MC, MCDisassembler, Object, Support, X86AsmPrinter, X86Info
  LLVMX86AsmPrinter # MC, Support, X86Utils
  LLVMX86Info # Support
  LLVMX86Utils # Core, Support
  LLVMCodeGen # Analysis, Core, MC, Scalar, Support, Target, TransformUtils
  LLVMipo
  LLVMScalarOpts
  LLVMInstCombine
  LLVMTransformUtils
  LLVMTarget # Analysis, MC, Core, Support
  LLVMAnalysis # Core, Support
  LLVMOption # Support
  LLVMMCDisassembler # MC, Support
  LLVMMCParser # MC, Support
  LLVMMC # Object, Support
  LLVMProfileData # Core, Support, Object
  LLVMObject # BitReader, Core, Support
  LLVMBitReader # Core, Support
  LLVMCore # BinaryFormat, Support
  LLVMBinaryFormat # Support
  LLVMSupport # Demangle
  LLVMDemangle
)

# Platform dependencies.
if( WIN32 )
  target_link_libraries(include-what-you-use
    PRIVATE
    shlwapi
    version # For clangDriver's MSVCToolchain
  )
elseif( UNIX )
  include(FindCurses)
  include(FindBacktrace)

  target_link_libraries(include-what-you-use
    PRIVATE
    pthread
    z
    ${Backtrace_LIBRARIES}
    ${CURSES_LIBRARIES}
    ${CMAKE_DL_LIBS}
  )
else()
  message(WARNING
    "Unknown system: ${CMAKE_SYSTEM_NAME}. "
    "No platform link-dependencies added.")
endif()

install(TARGETS include-what-you-use RUNTIME DESTINATION bin)
install(PROGRAMS fix_includes.py iwyu_tool.py DESTINATION bin)

# Install mapping files
file(GLOB MAPPING_FILES *.imp)
install(FILES ${MAPPING_FILES} DESTINATION share/include-what-you-use)
