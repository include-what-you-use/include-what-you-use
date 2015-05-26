//===--- deleted_implicit.cc - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that we don't crash when instantiating implicit methods, or rather,
// that we avoid instantiating implicit methods when they are to be considered
// deleted.

// http://en.cppreference.com/w/cpp/language/default_constructor#Deleted_implicitly-declared_default_constructor
// says C++11 considers the default constructor deleted if a type has a
// member of reference type.
// IWYU: class ...* does not declare any constructor to initialize its non-modifiable members
class ReferenceMember {
  int& ref_member;
};

// http://en.cppreference.com/w/cpp/language/copy_constructor#Deleted_implicitly-declared_copy_constructor
// says C++11 considers the copy constructor deleted if a type has a member of
// rvalue reference type.
// IWYU: class ...* does not declare any constructor to initialize its non-modifiable members
class RvalueReferenceMember {
  int&& ref_member;
};

// http://en.cppreference.com/w/cpp/language/move_constructor#Deleted_implicitly-declared_move_constructor
// says C++11 considers the move constructor deleted if a type has a non-static
// data member that cannot be moved (e.g. because its move ctor is deleted.)
class DeletedMoveCtor {
  DeletedMoveCtor(DeletedMoveCtor&&) = delete;
};

class ContainsDeletedMoveCtor {
  DeletedMoveCtor contained;
};

// http://en.cppreference.com/w/cpp/language/destructor#Deleted_implicitly-declared_destructor
// says C++11 considers the destructor deleted if a type has a base class with
// inaccessible or deleted destructor.
class PrivateDestructor {
  ~PrivateDestructor();
};

class Derived : public PrivateDestructor {
};

/**** IWYU_SUMMARY

(tests/cxx/deleted_implicit.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
