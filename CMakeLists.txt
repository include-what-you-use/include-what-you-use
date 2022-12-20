cmake_minimum_required(VERSION 3.4.3)

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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Synthesize clang-resource-headers target if necessary.
if (NOT TARGET clang-resource-headers)
  # Use only LLVM_VERSION_MAJOR for the resource directory structure; some
  # platforms include suffix in LLVM_VERSION.
  set(clang_headers_src "${LLVM_LIBRARY_DIR}/clang/${LLVM_VERSION_MAJOR}/include")
  set(clang_headers_dst "${CMAKE_BINARY_DIR}/lib/clang/${LLVM_VERSION_MAJOR}/include")

  file(GLOB_RECURSE in_files RELATIVE "${clang_headers_src}"
    "${clang_headers_src}/*"
  )

  set(out_files)
  foreach (file ${in_files})
    set(src "${clang_headers_src}/${file}")
    set(dst "${clang_headers_dst}/${file}")

    add_custom_command(OUTPUT "${dst}"
      DEPENDS "${src}"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${src}" "${dst}"
      COMMENT "Copying ${src}..."
    )
    list(APPEND out_files "${dst}")
  endforeach()

  add_custom_target(clang-resource-headers ALL DEPENDS ${out_files})
endif()

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
  iwyu_output.cc
  iwyu_path_util.cc
  iwyu_preprocessor.cc
  iwyu_regex.cc
  iwyu_verrs.cc
)

# Add a dependency on clang-resource-headers to ensure the builtin headers are
# available when IWYU is executed from the build dir.
add_dependencies(include-what-you-use clang-resource-headers)

# LLVM requires C++17, so follow suit.
set_target_properties(include-what-you-use PROPERTIES
  CXX_STANDARD_REQUIRED ON
  CXX_STANDARD 17
  CXX_EXTENSIONS OFF
)

target_compile_definitions(include-what-you-use PRIVATE
  ${LLVM_DEFINITIONS}
  IWYU_GIT_REV="${iwyu_git_rev}"
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

find_package(PythonInterp)
if (PYTHONINTERP_FOUND)
  # Map the various IWYU test scripts to CTest tests.
  enable_testing()

  function(iwyu_add_test name file)
    add_test(NAME ${name}
      COMMAND ${PYTHON_EXECUTABLE} run_iwyu_tests.py --run-test-file=${file}
      -- $<TARGET_FILE:include-what-you-use>
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
  endfunction()

  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} run_iwyu_tests.py --list-test-files
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE test_names_and_files
  )
  string(REPLACE "\n" ";" test_names_list ${test_names_and_files})
  foreach (test_name_and_file IN ITEMS ${test_names_list})
    string(REPLACE ":" ";" test_name_and_file ${test_name_and_file})
    iwyu_add_test(${test_name_and_file})
  endforeach()

  add_test(NAME fix_includes_test
    COMMAND ${PYTHON_EXECUTABLE} fix_includes_test.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
  add_test(NAME iwyu_tool_test
    COMMAND ${PYTHON_EXECUTABLE} iwyu_tool_test.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
endif()
