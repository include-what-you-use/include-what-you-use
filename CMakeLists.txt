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

set (IWYU_GENMAP_IMPS
  "${CMAKE_CURRENT_SOURCE_DIR}/gcc.symbols.imp"
  "${CMAKE_CURRENT_SOURCE_DIR}/gcc.libc.imp"
  "${CMAKE_CURRENT_SOURCE_DIR}/stl.c.headers.imp"
  "${CMAKE_CURRENT_SOURCE_DIR}/gcc.stl.headers.imp"
  "${CMAKE_CURRENT_SOURCE_DIR}/libstdcpp.symbols.imp"
  "${CMAKE_CURRENT_SOURCE_DIR}/freebsd.imp"
  )

set (IWYU_GENMAP_INCLUDES
  "gcc_libc_symbol_map.h"
  "gcc_libc_include_map.h"
  "stl_c_headers_include_map.h"
  "gcc_stl_headers_include_map.h"
  "libstdcpp_symbol_map.h"
  "freebsd_include_map.h"
  )

add_custom_command(
  OUTPUT ${IWYU_GENMAP_INCLUDES}
  DEPENDS ${IWYU_GENMAP_IMPS}
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/imp2array.py
  -i ${CMAKE_CURRENT_SOURCE_DIR}/gcc.symbols.imp
  -o gcc_libc_symbol_map.h
  -n libc_symbol_map
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/imp2array.py
  -i ${CMAKE_CURRENT_SOURCE_DIR}/gcc.libc.imp
  -o gcc_libc_include_map.h
  -n libc_include_map
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/imp2array.py
  -i ${CMAKE_CURRENT_SOURCE_DIR}/stl.c.headers.imp
  -o stl_c_headers_include_map.h
  -n stdlib_c_include_map
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/imp2array.py
  -i ${CMAKE_CURRENT_SOURCE_DIR}/gcc.stl.headers.imp
  -o gcc_stl_headers_include_map.h
  -n libstdcpp_include_map
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/imp2array.py
  -i ${CMAKE_CURRENT_SOURCE_DIR}/libstdcpp.symbols.imp
  -o libstdcpp_symbol_map.h
  -n libstdcpp_symbol_map
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/imp2array.py
  -i ${CMAKE_CURRENT_SOURCE_DIR}/freebsd.imp
  -o freebsd_include_map.h
  -n freebsd_include_map
  COMMENT "Generating the include maps"
  )

message(STATUS "IWYU: configuring for LLVM ${LLVM_VERSION}...")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

  
add_custom_target(genmap
  DEPENDS ${IWYU_GENMAP_IMPS}
  )

add_definitions(${LLVM_DEFINITIONS})
include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

set(LLVM_LINK_COMPONENTS
  Option
  Support
  X86AsmParser
  X86Desc
  X86Info
  )

add_llvm_executable(include-what-you-use
  ${IWYU_GENMAP_INCLUDES}
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

  # Put project in solution folder
  set_target_properties(include-what-you-use
    PROPERTIES FOLDER "Clang executables"
    )
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

  
