# This build leans on three variables from the parent project:
#
# - IWYU_SOURCE_DIR -- the source root directory of IWYU
# - IWYU_OBJECTS -- an OBJECT library of all IWYU object files
# - IWYU_LINK_LIBS -- a list of all IWYU link dependencies
#
# In an out-of-tree build, it needs to know where to find GTest, and so
# accepts IWYU_GTEST_INCLUDE_PATH and IWYU_GTEST_LIB_PATH. These are optional
# as GTest may be installed in a system root already.

function(warn_optional VARNAME)
  message(WARNING
    "${VARNAME} not provided, assuming GTest is available on default "
    "search path.")
endfunction(warn_optional)

if( CMAKE_SOURCE_DIR STREQUAL IWYU_SOURCE_DIR )
  # Out-of-tree build.
  if( DEFINED IWYU_GTEST_INCLUDE_PATH )
    include_directories(${IWYU_GTEST_INCLUDE_PATH})
  else()
    warn_optional("IWYU_GTEST_INCLUDE_PATH")
  endif()

  if( DEFINED IWYU_GTEST_LIB_PATH )
    link_directories(${IWYU_GTEST_LIB_PATH})
  else()
    warn_optional("IWYU_GTEST_LIB_PATH")
  endif()
else()
  # In-tree build, pick up GTest from LLVM.
  include_directories(${LLVM_MAIN_SRC_DIR}/utils/unittest/googletest/include)
endif()

# Configure GTest.
add_definitions(-DGTEST_HAS_RTTI=0)

# Need to be able to find the IWYU headers.
include_directories(${IWYU_SOURCE_DIR})

add_executable(include-what-you-use-tests
  $<TARGET_OBJECTS:IWYU_OBJECTS>
  # iwyu_include_picker_test.cc
  # iwyu_lexer_utils_test.cc
  # iwyu_output_test.cc
  iwyu_string_util_test.cc
  test_main.cc
)

target_link_libraries(include-what-you-use-tests
  gtest
  ${IWYU_LINK_LIBS}
)
