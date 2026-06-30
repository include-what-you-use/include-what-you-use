//===--- 1250b-i2.h - iwyu test ------* c++ *------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// A template that takes an iterator range.
template<class IteratorType>
void consume(IteratorType begin, IteratorType end);
