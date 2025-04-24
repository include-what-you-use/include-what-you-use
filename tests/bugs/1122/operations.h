#ifndef OPERATIONS_H
#define OPERATIONS_H

#define MULTIPLY_OP() \
int operator *(MyInt lhs, int rhs) { \
  return lhs.value * rhs; \
}

#endif  // OPERATIONS_H