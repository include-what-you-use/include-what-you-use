//===--- nested_class_friend_body.cc - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: --std=c++20

// Tests that a nested class is not forward-declared when it is used before its
// definition in the body of a friend function defined in the enclosing class.
// Like a method body, that body is complete-class context, so the later
// definition is visible to it. IWYU used to suggest the invalid
// 'struct Outer::Nested;'.

struct Outer {
  friend void Make() {
    Nested n;
    (void)n;
  }

  struct Nested {};
};

template <typename T>
struct Holder {
  T t;
};

template <typename T>
bool operator==(const Holder<T>& a, const Holder<T>& b) {
  return a.t == b.t;
}

// The defaulted comparison compares h, which instantiates the Holder
// operator== and uses Nested's operator==.
struct WithDefaulted {
  friend bool operator==(const WithDefaulted&, const WithDefaulted&) = default;

  struct Nested {
    int x;
    friend bool operator==(const Nested&, const Nested&) = default;
  };

  Holder<Nested> h;
};

bool Compare(const WithDefaulted& a, const WithDefaulted& b) {
  return a == b;
}

struct WithFriendTemplate {
  template <typename T>
  friend int Get(const WithFriendTemplate&, T t) {
    Nested n{t};
    return n.x;
  }

  struct Nested {
    int x;
  };
};

int CallGet() {
  return Get(WithFriendTemplate(), 1);
}

template <typename T>
struct OuterTpl {
  friend T Make(const OuterTpl&) {
    Nested n{T()};
    return n.t;
  }

  struct Nested {
    T t;
  };
};

int CallMake() {
  return Make(OuterTpl<int>());
}

/**** IWYU_SUMMARY

(tests/cxx/nested_class_friend_body.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
