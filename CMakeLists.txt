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
  if (POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
  endif()

  project(include-what-you-use)

  find_package(LLVM CONFIG REQUIRED)
  find_package(Clang CONFIG REQUIRED)

  list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})
  include(AddLLVM)
  include(HandleLLVMOptions)
endif()

message(STATUS "IWYU: configuring for LLVM ${LLVM_VERSION}...")

# The good default is given by the llvm toolchain installation itself, but still in
# case both static and shared libraries are available, allow to override that default.
option(IWYU_LINK_CLANG_DYLIB "Link against the clang dynamic library" ${CLANG_LINK_CLANG_DYLIB})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_definitions(${LLVM_DEFINITIONS})
include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
  )

# Synthesize a clang-resource-headers target for out-of-tree builds (in-tree
# already has it available by default)
if (NOT IWYU_IN_TREE)
  # Use only major.minor.patch for the resource directory structure; some
  # platforms include suffix in LLVM_VERSION.
  set(llvm_ver ${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}.${LLVM_VERSION_PATCH})
  set(clang_headers_src "${LLVM_LIBRARY_DIR}/clang/${llvm_ver}/include")
  set(clang_headers_dst "${CMAKE_BINARY_DIR}/lib/clang/${llvm_ver}/include")

  file(GLOB_RECURSE in_files RELATIVE "${clang_headers_src}"
    "${clang_headers_src}/*")

  set(out_files)
  foreach (file ${in_files})
    set(src "${clang_headers_src}/${file}")
    set(dst "${clang_headers_dst}/${file}")

    add_custom_command(OUTPUT "${dst}"
      DEPENDS "${src}"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${src}" "${dst}"
      COMMENT "Copying clang's ${file}...")
    list(APPEND out_files "${dst}")
  endforeach()

  add_custom_target(clang-resource-headers ALL DEPENDS ${out_files})
endif()

set(LLVM_LINK_COMPONENTS
  Option
  Support
  AllTargetsAsmParsers
  AllTargetsDescs
  AllTargetsInfos
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

# Add a dependency on clang-resource-headers to ensure the builtin headers are
# available when IWYU is executed from the build dir.
add_dependencies(include-what-you-use clang-resource-headers)

# LLVM requires C++14, so follow suit.
set_property(TARGET include-what-you-use PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET include-what-you-use PROPERTY CXX_STANDARD 14)
set_property(TARGET include-what-you-use PROPERTY CXX_EXTENSIONS OFF)

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

  # Enable bigobj support
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()

# Link dynamically or statically depending on user preference.
if (IWYU_LINK_CLANG_DYLIB)
  target_link_libraries(include-what-you-use PRIVATE clang-cpp)
else()
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
endif()

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
include(GNUInstallDirs)
install(TARGETS include-what-you-use RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(PROGRAMS fix_includes.py iwyu_tool.py DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install mapping files
file(GLOB MAPPING_FILES *.imp)
install(FILES ${MAPPING_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/include-what-you-use)

# Install man page on Unix-like systems
if (UNIX)
  install(FILES include-what-you-use.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
endif()

find_package(PythonInterp)
if(PYTHONINTERP_FOUND)
  enable_testing()

  function(ADD_IWYU_TEST NAME FILE)
    add_test(NAME ${NAME}
      COMMAND ${PYTHON_EXECUTABLE} run_iwyu_tests.py --run-test-file=${FILE} -- $<TARGET_FILE:include-what-you-use>
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  endfunction()

  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} run_iwyu_tests.py --list-test-files
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE TEST_NAMES_AND_FILES)
  string(REPLACE "\n" ";" TEST_NAMES_LIST ${TEST_NAMES_AND_FILES})
  foreach (TEST_NAME_AND_FILE IN ITEMS ${TEST_NAMES_LIST})
    string(REPLACE ":" ";" TEST_NAME_AND_FILE ${TEST_NAME_AND_FILE})
    ADD_IWYU_TEST(${TEST_NAME_AND_FILE})
  endforeach()

  add_test(NAME fix_includes_test
    COMMAND ${PYTHON_EXECUTABLE} fix_includes_test.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  add_test(NAME iwyu_tool_test
    COMMAND ${PYTHON_EXECUTABLE} iwyu_tool_test.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
