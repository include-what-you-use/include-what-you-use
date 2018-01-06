cmake_minimum_required(VERSION 3.4.3)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  message(STATUS "IWYU: out-of-tree configuration")

  if (POLICY CMP0048)
    # Silence CMP0048 warning about missing project VERSION.
    cmake_policy(SET CMP0048 NEW)
  endif()
  project(include-what-you-use)

  find_package(LLVM CONFIG REQUIRED)
  message(STATUS "IWYU: found LLVM (version ${LLVM_VERSION})")

  find_package(Clang CONFIG)
  if (Clang_FOUND)
    message(STATUS "IWYU: found a Clang")
  else()
    # Use workaround hack for broken Clang packaging, e.g.:
    # https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=862328
    include("${CMAKE_CURRENT_SOURCE_DIR}/clang-skeleton.cmake")
  endif()
else()
  message(STATUS "IWYU: in-tree configuration")
endif()

# LLVM/Clang build settings
list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})
include(AddLLVM)
include(HandleLLVMOptions)

add_definitions(${LLVM_DEFLINITIONS})
include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
  )

# IWYU build settings
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

if (MSVC)
  # Disable warnings for IWYU, and disable exceptions in MSVC's STL.
  add_definitions(
    -wd4722 # Suppress ''destructor'' : destructor never returns, potential memory leak
    -D_HAS_EXCEPTIONS=0
    )

  # Enable bigobj support and sane C++ exception semantics.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /EHsc")
elseif (MINGW)
  # Work around 'too many sections' error with MINGW/GCC
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj")
endif()

# Link dependencies.
set(IWYU_CLANG_DEPS
  clangFrontend
  clangDriver
  clangSema
  clangAST
  clangBasic
  )

set(IWYU_LLVM_DEPS
  LLVMX86AsmParser
  LLVMX86Desc
  LLVMX86Info
  LLVMSupport
  )

target_link_libraries(include-what-you-use
  PRIVATE
  ${IWYU_CLANG_DEPS}
  ${IWYU_LLVM_DEPS}
  )

if (WIN32)
  target_link_libraries(include-what-you-use
    PRIVATE
    shlwapi
    )
endif()

# Pick up Git revision so we can report it in version information.
if (NOT DEFINED IWYU_GIT_REV)
  find_package(Git MODULE)

  if (Git_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      OUTPUT_VARIABLE IWYU_GIT_REV
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    add_definitions(-DIWYU_GIT_REV="${IWYU_GIT_REV}")
  endif()
endif()

if (NOT IWYU_GIT_REV)
  message(WARNING "IWYU: Git version info not found, "
    "DO NOT release from this build tree!")
endif()

# Install programs
install(TARGETS include-what-you-use RUNTIME DESTINATION bin)
install(PROGRAMS fix_includes.py iwyu_tool.py DESTINATION bin)

# Install mapping files
file(GLOB MAPPING_FILES *.imp)
install(FILES ${MAPPING_FILES} DESTINATION share/include-what-you-use)
