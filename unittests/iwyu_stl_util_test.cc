//===--- iwyu_stl_util_test.cc - test iwyu_stl_util.h ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests for the iwyu_stl_util module.

#include "iwyu_stl_util.h"

#include <map>
#include <set>
#include <string>
#include <vector>

#include "gtest/gtest.h"

namespace include_what_you_use {
using std::map;
using std::set;
using std::string;
using std::vector;

namespace {

TEST(IwyuSTLUtilTest, UnionSetSemantics) {
  // Make sure we eliminate duplicates
  set<int> s1{1, 2};
  set<int> s2{1, 2};
  EXPECT_EQ(set<int>({1, 2}), Union(s1, s2));
  set<int> s3{1, 2};
  EXPECT_EQ(set<int>({1, 2}), Union(s1, s2, s3));
}

TEST(IwyuSTLUtilTest, Union) {
  set<int> s1{1, 2, 3};
  set<int> s2{4, 5, 6};
  set<int> s3{7, 8, 9};
  set<int> s4{10, 11, 12};

  // Union requires at least one set.
  EXPECT_EQ(set<int>({1, 2, 3}), Union(s1));
  EXPECT_EQ(set<int>({1, 2, 3, 4, 5, 6}), Union(s1, s2));

  // Odd number of sets.
  EXPECT_EQ(set<int>({1, 2, 3, 4, 5, 6, 7, 8, 9}), Union(s1, s2, s3));

  // Even number of sets >2.
  EXPECT_EQ(set<int>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}),
            Union(s1, s2, s3, s4));

  // Union of set rvalues.
  EXPECT_EQ(set<int>({1, 2, 3, 4, 5, 6}), Union(std::move(s1), std::move(s2)));

  // Union of mixed lvalues and rvalues.
  EXPECT_EQ(set<int>({7, 8, 9, 10, 11, 12}), Union(std::move(s3), s4));
}

TEST(IwyuSTLUtilTest, ContainsKeyPresent) {
  map<string, int> m{{"a", 1}, {"b", 2}};
  EXPECT_TRUE(ContainsKey(m, string("a")));
  EXPECT_TRUE(ContainsKey(m, string("b")));
}

TEST(IwyuSTLUtilTest, ContainsKeyAbsent) {
  map<string, int> m{{"a", 1}};
  EXPECT_FALSE(ContainsKey(m, string("z")));
}

TEST(IwyuSTLUtilTest, ContainsKeyEmptyContainer) {
  map<int, int> m;
  EXPECT_FALSE(ContainsKey(m, 1));
}

TEST(IwyuSTLUtilTest, ContainsKeyInSet) {
  set<int> s{10, 20, 30};
  EXPECT_TRUE(ContainsKey(s, 20));
  EXPECT_FALSE(ContainsKey(s, 99));
}

TEST(IwyuSTLUtilTest, ContainsValuePresent) {
  vector<int> v{1, 2, 3};
  EXPECT_TRUE(ContainsValue(v, 2));
}

TEST(IwyuSTLUtilTest, ContainsValueAbsent) {
  vector<int> v{1, 2, 3};
  EXPECT_FALSE(ContainsValue(v, 4));
}

TEST(IwyuSTLUtilTest, ContainsValueEmpty) {
  vector<string> v;
  EXPECT_FALSE(ContainsValue(v, string("x")));
}

TEST(IwyuSTLUtilTest, ContainsKeyValuePresent) {
  map<string, int> m{{"a", 1}, {"b", 2}};
  EXPECT_TRUE(ContainsKeyValue(m, string("a"), 1));
}

TEST(IwyuSTLUtilTest, ContainsKeyValueWrongValue) {
  map<string, int> m{{"a", 1}};
  EXPECT_FALSE(ContainsKeyValue(m, string("a"), 99));
}

TEST(IwyuSTLUtilTest, ContainsKeyValueMissingKey) {
  map<string, int> m{{"a", 1}};
  EXPECT_FALSE(ContainsKeyValue(m, string("z"), 1));
}

TEST(IwyuSTLUtilTest, ContainsAnyKeyMatch) {
  map<int, string> m{{1, "a"}, {2, "b"}, {3, "c"}};
  set<int> keys{2, 5};
  EXPECT_TRUE(ContainsAnyKey(m, keys));
}

TEST(IwyuSTLUtilTest, ContainsAnyKeyNoMatch) {
  map<int, string> m{{1, "a"}, {2, "b"}};
  set<int> keys{10, 20};
  EXPECT_FALSE(ContainsAnyKey(m, keys));
}

TEST(IwyuSTLUtilTest, ContainsAnyKeyEmptyKeys) {
  map<int, string> m{{1, "a"}};
  set<int> keys;
  EXPECT_FALSE(ContainsAnyKey(m, keys));
}

TEST(IwyuSTLUtilTest, GetOrDefaultHit) {
  map<string, int> m{{"a", 42}};
  EXPECT_EQ(42, GetOrDefault(m, string("a"), -1));
}

TEST(IwyuSTLUtilTest, GetOrDefaultMiss) {
  map<string, int> m{{"a", 42}};
  EXPECT_EQ(-1, GetOrDefault(m, string("z"), -1));
}

TEST(IwyuSTLUtilTest, GetOrDefaultEmpty) {
  map<int, int> m;
  EXPECT_EQ(0, GetOrDefault(m, 1, 0));
}

TEST(IwyuSTLUtilTest, FindInMapHit) {
  map<string, int> m{{"a", 1}, {"b", 2}};
  const int* val = FindInMap(&m, string("b"));
  ASSERT_NE(nullptr, val);
  EXPECT_EQ(2, *val);
}

TEST(IwyuSTLUtilTest, FindInMapMiss) {
  map<string, int> m{{"a", 1}};
  const int* val = FindInMap(&m, string("z"));
  EXPECT_EQ(nullptr, val);
}

TEST(IwyuSTLUtilTest, FindInMapMutable) {
  map<string, int> m{{"a", 1}};
  int* val = FindInMap(&m, string("a"));
  ASSERT_NE(nullptr, val);
  *val = 99;
  EXPECT_EQ(99, m["a"]);
}

TEST(IwyuSTLUtilTest, RemoveAllFromSet) {
  set<int> target{1, 2, 3, 4, 5};
  set<int> to_remove{2, 4};
  RemoveAllFrom(to_remove, &target);
  EXPECT_EQ(set<int>({1, 3, 5}), target);
}

TEST(IwyuSTLUtilTest, RemoveAllFromNoOverlap) {
  set<int> target{1, 2, 3};
  set<int> to_remove{10, 20};
  RemoveAllFrom(to_remove, &target);
  EXPECT_EQ(set<int>({1, 2, 3}), target);
}

TEST(IwyuSTLUtilTest, RemoveAllFromEmpty) {
  set<int> target{1, 2, 3};
  set<int> to_remove;
  RemoveAllFrom(to_remove, &target);
  EXPECT_EQ(set<int>({1, 2, 3}), target);
}

TEST(IwyuSTLUtilTest, InsertAllIntoSet) {
  set<int> source{3, 4, 5};
  set<int> target{1, 2, 3};
  InsertAllInto(source, &target);
  EXPECT_EQ(set<int>({1, 2, 3, 4, 5}), target);
}

TEST(IwyuSTLUtilTest, InsertAllIntoEmpty) {
  set<int> source;
  set<int> target{1, 2};
  InsertAllInto(source, &target);
  EXPECT_EQ(set<int>({1, 2}), target);
}

TEST(IwyuSTLUtilTest, ExtendVector) {
  vector<int> target{1, 2};
  vector<int> source{3, 4, 5};
  Extend(&target, source);
  EXPECT_EQ(vector<int>({1, 2, 3, 4, 5}), target);
}

TEST(IwyuSTLUtilTest, ExtendEmpty) {
  vector<int> target{1, 2};
  vector<int> source;
  Extend(&target, source);
  EXPECT_EQ(vector<int>({1, 2}), target);
}

TEST(IwyuSTLUtilTest, GetUniqueEntriesNoDuplicates) {
  vector<int> v{1, 2, 3};
  EXPECT_EQ(vector<int>({1, 2, 3}), GetUniqueEntries(v));
}

TEST(IwyuSTLUtilTest, GetUniqueEntriesWithDuplicates) {
  vector<int> v{3, 1, 2, 1, 3, 4};
  EXPECT_EQ(vector<int>({3, 1, 2, 4}), GetUniqueEntries(v));
}

TEST(IwyuSTLUtilTest, GetUniqueEntriesEmpty) {
  vector<int> v;
  EXPECT_EQ(vector<int>(), GetUniqueEntries(v));
}

}  // namespace
}  // namespace include_what_you_use
