cmake_minimum_required(VERSION 3.20.0)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(iwyu_standalone_build ON)
  message(STATUS "IWYU: standalone build")
else()
  set(iwyu_standalone_build OFF)
  message(STATUS "IWYU: build as part of LLVM")
endif()

if (iwyu_standalone_build)
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

  set(iwyu_llvm_dir ${LLVM_DIR})
  set(iwyu_include_dirs
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
  )
else()
  set(iwyu_llvm_dir ${CMAKE_SOURCE_DIR})
  set(iwyu_include_dirs
    ${LLVM_SOURCE_DIR}/include
    ${LLVM_EXTERNAL_CLANG_SOURCE_DIR}/include
    ${LLVM_BINARY_DIR}/include
    ${LLVM_BINARY_DIR}/tools/clang/include
  )
endif()

message(STATUS
  "IWYU: configuring for LLVM ${LLVM_VERSION} from ${iwyu_llvm_dir}")

# The good default is given by the llvm toolchain installation itself, but still
# in case both static and shared libraries are available, allow to override that
# default.
option(IWYU_LINK_CLANG_DYLIB
  "Link against the clang dynamic library"
  ${CLANG_LINK_CLANG_DYLIB}
)

# IWYU needs to know where to find Clang builtin headers (stddef.h, stdint.h,
# etc). The builtin headers are shipped in the Clang resource directory.
# You can configure IWYU's resource directory lookup using two options:
#
# * IWYU_RESOURCE_RELATIVE_TO: which executable to serve as the base path for
#   relative resource dir lookups ('iwyu' or 'clang')
# * IWYU_RESOURCE_DIR: a path relative to the executable above, resolved at
#   runtime (analogous to Clang's CLANG_RESOURCE_DIR)
#
# IWYU configures with IWYU_RESOURCE_RELATIVE_TO=clang and IWYU_RESOURCE_DIR=""
# by default, because the builtin headers are always available next to Clang for
# development and local test.
#
# When packaging IWYU, you can:
#
# * configure it relative to 'clang' and add a package dependency from IWYU to
#   Clang to ensure the headers are available
# * configure it relative to 'iwyu' and add custom package install steps to copy
#   the headers from the Clang tree into the IWYU install tree
#
# Generally if you override the defaults, you are responsible for making sure
# the headers are where you said they would be.
if (NOT IWYU_RESOURCE_RELATIVE_TO)
  # This might look counter-intuitive, but it is most likely what you want for
  # local developer builds and packaging builds. See above for details.
  set(IWYU_RESOURCE_RELATIVE_TO "clang")
endif()

if (IWYU_RESOURCE_RELATIVE_TO STREQUAL "clang")
  # Point resource bin path to the clang target's output name if nothing else
  # specified (e.g. /usr/lib/llvm-<ver>/bin/clang).
  set(iwyu_resource_binary_path $<TARGET_FILE:clang>)
elseif (IWYU_RESOURCE_RELATIVE_TO STREQUAL "iwyu")
  # Leave bin path empty so IWYU can resolve its own path at runtime.
  set(iwyu_resource_binary_path "")
else()
  message(FATAL_ERROR "Invalid IWYU_RESOURCE_RELATIVE_TO value: "
    "'${IWYU_RESOURCE_RELATIVE_TO}'. Must be 'iwyu' or 'clang'.")
endif()

if (NOT IWYU_RESOURCE_DIR)
  # CLANG_RESOURCE_DIR is not exported so will be empty for standalone builds.
  # It might, but usually doesn't, have a value in non-standalone builds, where
  # IWYU is part of LLVM's CMake system.
  set(iwyu_resource_dir "${CLANG_RESOURCE_DIR}")
else()
  set(iwyu_resource_dir "${IWYU_RESOURCE_DIR}")
endif()

message(STATUS
  "IWYU: Resource dir will be computed at runtime with IWYU_RESOURCE_DIR="
  "'${iwyu_resource_dir}' relative to '${IWYU_RESOURCE_RELATIVE_TO}'")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Pick up Git revision so we can report it in version information.
include(FindGit)
if (GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
  execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE iwyu_git_rev
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
else()
  message(WARNING "IWYU Git version not found, DO NOT release from this tree!")
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
  iwyu_main.cc
  iwyu_output.cc
  iwyu_path_util.cc
  iwyu_port.cc
  iwyu_preprocessor.cc
  iwyu_regex.cc
  iwyu_verrs.cc
)

# Add a dependency on clang-resource-headers if it exists, to ensure the builtin
# headers are available where Clang/IWYU expects them after build.
# This should only have any effect in non-standalone builds.
if (TARGET clang-resource-headers)
  add_dependencies(include-what-you-use clang-resource-headers)
endif()

# LLVM requires C++17, so follow suit.
set_target_properties(include-what-you-use PROPERTIES
  CXX_STANDARD_REQUIRED ON
  CXX_STANDARD 17
  CXX_EXTENSIONS OFF
)

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})

target_compile_definitions(include-what-you-use PRIVATE
  ${LLVM_DEFINITIONS_LIST}
  IWYU_GIT_REV="${iwyu_git_rev}"
  IWYU_RESOURCE_BINARY_PATH="${iwyu_resource_binary_path}"
  IWYU_RESOURCE_DIR="${iwyu_resource_dir}"
)
target_include_directories(include-what-you-use PRIVATE
  ${iwyu_include_dirs}
)

if (MINGW)
  target_compile_options(include-what-you-use PRIVATE
    # Work around 'too many sections' error with MINGW/GCC.
    -Wa,-mbig-obj
  )
endif()

if (MSVC)
  target_compile_options(include-what-you-use PRIVATE
    # Suppress ''destructor'' : destructor never returns, potential memory leak.
    /wd4722
    # Disable exceptions in MSVC STL.
    /D_HAS_EXCEPTIONS=0
    # Suppress C1128: number of sections exceeded object file format limit.
    /bigobj
  )
endif()

# Link dynamically or statically depending on user preference.
if (IWYU_LINK_CLANG_DYLIB)
  target_link_libraries(include-what-you-use PRIVATE clang-cpp)
else()
  target_link_libraries(include-what-you-use PRIVATE
    clangBasic
    clangLex
    clangAST
    clangSema
    clangFrontend
    clangFrontendTool
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

    clangToolingInclusionsStdlib
  )
endif()

# Add platform-specific link dependencies.
if (WIN32)
  target_link_libraries(include-what-you-use PRIVATE
    shlwapi  # for PathMatchSpecA
  )
endif()

# Install programs.
include(GNUInstallDirs)
install(TARGETS
  include-what-you-use
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(PROGRAMS
  fix_includes.py
  iwyu_tool.py
  DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install mapping files.
file(GLOB mapping_files *.imp)
install(FILES
  ${mapping_files}
  DESTINATION ${CMAKE_INSTALL_DATADIR}/include-what-you-use
)

# Install man page on Unix-like systems.
if (UNIX)
  install(FILES
    include-what-you-use.1
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
  )
endif()

find_package(Python3 COMPONENTS Interpreter)
if (Python3_Interpreter_FOUND)
  # Map the various IWYU test scripts to CTest tests.
  enable_testing()

  function(iwyu_add_test name file)
    add_test(NAME ${name}
      COMMAND ${Python3_EXECUTABLE} run_iwyu_tests.py --run-test-file=${file}
      -- $<TARGET_FILE:include-what-you-use>
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_tests_properties(${name} PROPERTIES SKIP_RETURN_CODE 77)
  endfunction()

  execute_process(
    COMMAND ${Python3_EXECUTABLE} run_iwyu_tests.py --list-test-files
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE test_names_and_files
  )
  string(REPLACE "\n" ";" test_names_list ${test_names_and_files})
  foreach (test_name_and_file IN ITEMS ${test_names_list})
    string(REPLACE ":" ";" test_name_and_file ${test_name_and_file})
    iwyu_add_test(${test_name_and_file})
  endforeach()

  add_test(NAME fix_includes_test
    COMMAND ${Python3_EXECUTABLE} fix_includes_test.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
  add_test(NAME iwyu_tool_test
    COMMAND ${Python3_EXECUTABLE} iwyu_tool_test.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
endif()
