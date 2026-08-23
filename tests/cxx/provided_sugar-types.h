//===--- provided_sugar-types.h - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

class DecltypeReturn;
class DecltypeArgument;

extern DecltypeReturn decltype_result;
extern DecltypeArgument decltype_argument;

class TypeofReturn;
class TypeofArgument;

extern TypeofReturn typeof_result;
extern TypeofArgument typeof_argument;

class SameFileReturn;

extern SameFileReturn same_file_result;

class NestedSugarReturn;

extern NestedSugarReturn nested_sugar_result;

class TemplateSugarReturn;

extern TemplateSugarReturn template_sugar_result;
