cmake_minimum_required(VERSION 3.4.3)

if( POLICY CMP0048 )
  # Silence CMP0048 warning about missing project VERSION.
  cmake_policy(SET CMP0048 NEW)
endif()

project(include-what-you-use)

if( CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR )
  message(STATUS "IWYU out-of-tree configuration")
  find_package(LLVM CONFIG REQUIRED)
  find_package(Clang CONFIG REQUIRED)
  list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})
else()
  message(STATUS "IWYU in-tree configuration")
endif()

include(AddLLVM)
include(HandleLLVMOptions)

add_definitions(${LLVM_DEFINITIONS})
include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
  )

add_llvm_executable(include-what-you-use
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

# LLVM/Clang dependencies.
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

# Pick up Git revision so we can report it in version information.
if( NOT DEFINED IWYU_GIT_REV )
  find_package(Git MODULE)

  if( Git_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git" )
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      OUTPUT_VARIABLE IWYU_GIT_REV
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    add_definitions(-DIWYU_GIT_REV="${IWYU_GIT_REV}")
  endif()
endif()

if( NOT IWYU_GIT_REV )
  message(WARNING "IWYU Git version info not found, "
    "DO NOT release from this build tree!")
endif()

# Install programs
install(TARGETS include-what-you-use RUNTIME DESTINATION bin)
install(PROGRAMS fix_includes.py iwyu_tool.py DESTINATION bin)

# Install mapping files
file(GLOB MAPPING_FILES *.imp)
install(FILES ${MAPPING_FILES} DESTINATION share/include-what-you-use)
