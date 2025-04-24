#ifndef MY_INT_H
#define MY_INT_H

#include "operations.h"

struct MyInt {
  int value;
};

MyInt MakeMyInt(int value) {
  return {value};
}

MULTIPLY_OP()

#endif  // MY_INT_H