cmake_minimum_required(VERSION 3.4.3)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  message(STATUS "IWYU: out-of-tree configuration")
  set(IWYU_IN_TREE OFF)
else()
  message(STATUS "IWYU: in-tree configuration")
  set(IWYU_IN_TREE ON)
endif()

if (NOT IWYU_IN_TREE)
  cmake_policy(SET CMP0048 NEW)
  project(include-what-you-use)

  find_package(LLVM CONFIG REQUIRED)
  find_package(Clang CONFIG REQUIRED)

  list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})
  include(AddLLVM)
  include(HandleLLVMOptions)
endif()

message(STATUS "IWYU: configuring for LLVM ${LLVM_VERSION}...")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_definitions(${LLVM_DEFINITIONS})
include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
  )

set(LLVM_LINK_COMPONENTS
  Option
  Support
  X86AsmParser
  X86Desc
  X86Info
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

if (IWYU_IN_TREE)
  # Add a dependency on clang-headers to ensure the builtin headers are
  # available when IWYU is executed from the build dir.
  # The clang-headers target is only available in in-tree builds.
  add_dependencies(include-what-you-use clang-headers)
endif()

if (MINGW)
  # Work around 'too many sections' error with MINGW/GCC
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj")
endif()

if (MSVC)
  # Disable warnings for IWYU, and disable exceptions in MSVC's STL.
  add_definitions(
    -wd4722 # Suppress ''destructor'' : destructor never returns, potential memory leak
    -D_HAS_EXCEPTIONS=0
    )

  # Enable bigobj support and sane C++ exception semantics.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /EHsc")
endif()

target_link_libraries(include-what-you-use
  PRIVATE
  clangBasic
  clangLex
  clangAST
  clangSema
  clangFrontend
  clangDriver

  # Revision [1] in clang moved PCHContainerOperations from Frontend
  # to Serialization, but this broke builds that set
  # -DBUILD_SHARED_LIBS=on.  Revision [2] is a followup that works
  # around the issue by adding an explicit dependency on Serialization
  # wherever there was a dependency on Frontend.  Since we depend on
  # Frontend, we need an explicit dependency on Serialization too.
  # [1] https://llvm.org/viewvc/llvm-project?view=revision&revision=348907
  # [2] https://llvm.org/viewvc/llvm-project?view=revision&revision=348915
  clangSerialization
  )

# Platform dependencies.
if (WIN32)
  target_link_libraries(include-what-you-use
    PRIVATE
    shlwapi  # For PathMatchSpecA
    )
endif()

# Pick up Git revision so we can report it in version information.
include(FindGit)
if (GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
  execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE IWYU_GIT_REV
    OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
  message(STATUS "Warning: IWYU Git version info not found, DO NOT release "
                 "from this build tree!")
endif()
add_definitions(-DIWYU_GIT_REV="${IWYU_GIT_REV}")

# Install programs
install(TARGETS include-what-you-use RUNTIME DESTINATION bin)
install(PROGRAMS fix_includes.py iwyu_tool.py DESTINATION bin)

# Install mapping files
file(GLOB MAPPING_FILES *.imp)
install(FILES ${MAPPING_FILES} DESTINATION share/include-what-you-use)
