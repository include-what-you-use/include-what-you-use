//===--- macro_location_tpl-i1.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Forward-declare the primary templates as a signal that specialization uses
// belong in the expansion location.
template <typename>
void expansion_template();

template <typename>
class ExpansionTemplate;

// expansion_template should be attributed to expansion loc.
#define FUNC_TEMPLATE_SPEC_EXPANSION expansion_template<int>()

// spelling_template should be attributed to this file, because there's no
// forward-declare hint.
#define FUNC_TEMPLATE_SPEC_SPELLING spelling_template<int>();

// ExpansionTemplate should be attributed to expansion loc.
#define CLASS_TEMPLATE_SPEC_EXPANSION ExpansionTemplate<int>().method();

// SpellingTemplate should be attributed to this file, because there's no
// forward-declare hint.
#define CLASS_TEMPLATE_SPEC_SPELLING SpellingTemplate<int>().method();
