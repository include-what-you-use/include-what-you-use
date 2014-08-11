//===--- prefix_header_attribution.cc - test input file for iwyu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that when IWYU attributes macros and placement operator new to
// providing headers, it preserves enough information to detect if providing
// header is prefix header.
//
// Test requirements:
// - use --prefix_header_includes=remove to force is_prefix_header check for
//   all includes;
// - don't include providing headers directly.  Because otherwise IWYU checks
//   is_prefix_header for existing inclusion directive and ignores all
//   information preserved during symbol-to-file attribution.

// Test macro.
// IWYU: MACRO_IN_PREFIX_HEADER is...*prefix_header_attribution-i1.h
MACRO_IN_PREFIX_HEADER;

// Test placement operator new.
template<typename T> void CallPlacementNew(T *t) {
  // IWYU: operator new is...*<new>
  new (t) T();
}

/**** IWYU_SUMMARY

(tests/cxx/prefix_header_attribution.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
