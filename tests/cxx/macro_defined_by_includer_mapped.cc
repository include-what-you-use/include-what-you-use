//===--- macro_defined_by_includer_mapped.cc - test input file for iwyu ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -Xiwyu --mapping_file=tests/cxx/macro_defined_by_includer_mapped.imp

// Tests a header that uses a macro defined by its includer when the header
// is also mapped to a public header in some other way: exported by the
// includer, or private with a mapping to its public header.  Those mappings
// still apply to the declarations that do not depend on the macro.

#include "tests/cxx/macro_defined_by_includer_mapped-d1.h"
#include "tests/cxx/macro_defined_by_includer_mapped-d2.h"
#include "tests/cxx/macro_defined_by_includer_mapped-d3.h"

// -d1.h exports -i1.h, and defines a macro that -i1.h uses, in the style of
// absl/numeric/int128.h and int128_have_intrinsic.inc.
bool exported(ExportedValue a, ExportedValue b) {
  return a == b;
}
ExportedValue exported_value = MakeExportedValue('x');

// -i2.h is private to -d2.h by pragma and checks the include guard of -d2.h,
// in the style of glibc <bits/*.h>.
PragmaPrivateBase pragma_private_base;
PragmaPrivateFeature pragma_private_feature;

// -i3.h is private to -d3.h by a mapping file and checks the include guard
// of -d3.h.
MappedPrivateBase mapped_private_base;
MappedPrivateFeature mapped_private_feature;

/**** IWYU_SUMMARY

(tests/cxx/macro_defined_by_includer_mapped.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
