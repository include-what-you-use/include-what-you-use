#ifndef __INCLUDE2_H__
#define __INCLUDE2_H__

#define __disabled_things
#include "include1.h"
#undef __disabled_things

struct something2 {
  char text[32];
};
#endif  //__INCLUDE2_H__
