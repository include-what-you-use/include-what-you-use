//===--- keep_moc-i1.h - test input file for iwyu -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Fake some Qt-isms to be able to declare a QObject-like class below.
#define Q_OBJECT
#define signals public
class QObject {};

// Make sure we don't remove .moc even if, as in this case, it's empty.
#include "tests/cxx/keep_moc.moc"

class QObjectLike : public QObject {
  Q_OBJECT

signals:
  void Hello();
};

#undef Q_OBJECT
#undef signals

/**** IWYU_SUMMARY

(tests/cxx/keep_moc-i1.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */

