//===--- ms_inline_asm.cc - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This file is not strictly an IWYU test, it just checks that the parser
// doesn't choke on Microsoft inline assembly on any of our target platforms.
// Requires -fms-extensions.

int main() {
  int r;
  __asm {
    // TODO: Add a use here, e.g. by using IndirectClass::statica
    // when/if we ever support IWYU analysis inside inline assembly.
    mov ecx, 0
    mov r, ecx
  };

  return r;
}

/**** IWYU_SUMMARY

(tests/cxx/ms_inline_asm.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
