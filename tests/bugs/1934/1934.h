//===--- 1934.h - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#define FOR_EACH_ANIMAL(ACTION) \
  ACTION(NONE, CAT)             \
  ACTION(CAT, LION)             \
  ACTION(CAT, TIGER)

class iwyu1934
{
public:
  enum Animal : unsigned
  {
    NONE,
#define ANIMAL_ENUM(P, A) A,
    FOR_EACH_ANIMAL(ANIMAL_ENUM)
#undef DIAGNOSTIC_ENUM
    COUNT
  };
  constexpr static unsigned Count = static_cast<unsigned>(COUNT);

  struct Relation
  {
    Animal Parent;
  };

  // The problem seems to be some combination of this member and the macros
  constexpr static Animal
    Relations[Count] = {
      NONE, // NONE
#define RELATION(P, A) P,
      FOR_EACH_ANIMAL(RELATION)
#undef DIAGNOSTIC_CATEGORY_INFO
    };
};

/**** IWYU_SUMMARY

(tests/bugs/1934/1934.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
