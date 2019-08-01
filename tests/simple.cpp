#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <chrono>
#include <deque>
#include "gtest/gtest.h"
#include "caps.h"
#include "leb128.h"

using namespace std;
using namespace std::chrono;
using namespace rokid;

template <typename T, int32_t M>
void testLeb128(uint8_t* buf, uint32_t size) {
  auto p = buf;
  uint32_t sz;
  deque<T> nums;
  int32_t m{M};
  while (true) {
    T v = rand() * m;
    m = -m;
    sz = size - (p - buf);
    try {
      p = leb128Write(v, p, sz);
    } catch (out_of_range& e) {
      break;
    }
    nums.push_back(v);
  }

  size = p - buf;
  p = buf;
  T v;
  while (p - buf < size) {
    p = leb128Read(p, v);
    EXPECT_EQ(v, nums.front());
    nums.pop_front();
  }
  EXPECT_EQ(p - buf, size);
  EXPECT_TRUE(nums.empty());
}

template <typename T, int32_t M>
void testULeb128(uint8_t* buf, uint32_t size) {
  auto p = buf;
  uint32_t sz;
  deque<T> nums;
  int32_t m{M};
  while (true) {
    T v = rand() * m;
    sz = size - (p - buf);
    try {
      p = uleb128Write(v, p, sz);
    } catch (out_of_range& e) {
      break;
    }
    nums.push_back(v);
  }

  size = p - buf;
  p = buf;
  T v;
  while (p - buf < size) {
    p = uleb128Read(p, v);
    EXPECT_EQ(v, nums.front());
    nums.pop_front();
  }
  EXPECT_EQ(p - buf, size);
  EXPECT_TRUE(nums.empty());
}

TEST(TestLeb128, simple) {
  srand(duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count());
  uint8_t buf[4096];
  testLeb128<int32_t, 1>(buf, sizeof(buf));
  testLeb128<int64_t, 1000000>(buf, sizeof(buf));
  testULeb128<uint32_t, 1>(buf, sizeof(buf));
  testULeb128<uint64_t, 1000000>(buf, sizeof(buf));
}

static void writeCaps(Caps& caps) {
  caps << 1;
  caps << true;
  caps << "hello";
  caps << string("world");
  caps << (float)0.1;
  caps << (int64_t)10000LL;
  caps << (double)1.1;
  caps << vector<char>{ 'f', 'o', 'o' };
  Caps sub;
  sub.write();
  caps << sub;
}

static void readCaps(const Caps& caps) {
  try {
    int32_t t = caps.at(0);
    EXPECT_EQ(t, 1);
    EXPECT_THROW(t = caps.at(2), Caps::type_error);
    string s = caps.at(2);
    EXPECT_EQ(s, "hello");
    Caps c = caps.at(8);
    EXPECT_EQ(c.size(), 1);
    EXPECT_EQ(c.at(0).isVoid(), true);
    bool b = caps.at(1);
    int8_t i1;
    uint8_t u1;
    u1 = caps.at(1);
    EXPECT_THROW(i1 = caps.at(1), Caps::type_error);
  } catch (exception& e) {
    FAIL() << e.what();
  }
}

static void iterateCaps(Caps::iterator& it) {
  int32_t i;
  bool b;
  string s1, s2;
  float f;
  int64_t l;
  double d;
  vector<char> bin;
  Caps sub;

  it >> i;
  it >> b;
  it >> s1;
  it >> s2;
  it >> f;
  it >> l;
  it >> d;
  it >> bin;
  it >> sub;
  EXPECT_EQ(i, 1);
  EXPECT_EQ(b, true);
  EXPECT_EQ(s1, "hello");
  EXPECT_EQ(s2, "world");
  EXPECT_EQ(f, (float)0.1);
  EXPECT_EQ(l, 10000LL);
  EXPECT_EQ(d, (double)1.1);
  EXPECT_EQ(bin.size(), 3);
  EXPECT_EQ(memcmp(bin.data(), "foo", 3), 0);
  EXPECT_EQ(sub.size(), 1);
  EXPECT_EQ(sub[0].isVoid(), true);
}

static void iterateCaps(const Caps& caps) {
  auto it = caps.iterate();
  iterateCaps(it);
}

TEST(TestCaps, base) {
  Caps caps;
  writeCaps(caps);
  readCaps(caps);
  try {
    char buf[256];
    auto sz = caps.serialize(buf, sizeof(buf));
    Caps ncaps;
    ncaps.parse(buf, sz);
    readCaps(ncaps);
  } catch (exception& e) {
    FAIL() << e.what();
  }
}

TEST(TestCaps, iterate) {
  Caps caps;
  writeCaps(caps);
  Caps::iterator it;
  try {
    Caps ncaps;
    char buf[256];
    auto sz = caps.serialize(buf, sizeof(buf));
    ncaps.parse(buf, sz);
    iterateCaps(ncaps);
    iterateCaps(ncaps);
    iterateCaps(ncaps);
    it = ncaps.iterate();
    iterateCaps(it);
    EXPECT_EQ(it.hasNext(), false);
    it = ncaps.iterate();
  } catch (exception& e) {
    FAIL() << e.what();
  }
  EXPECT_THROW(it.next(), out_of_range);
}

TEST(TestCaps, dump) {
  Caps caps, ncaps;
  writeCaps(caps);
  char abuf[256];
  char bbuf[256];
  char cbuf[4];
  uint32_t c1, c2;

  try {
    auto sz = caps.serialize(abuf, sizeof(abuf));
    ncaps.parse(abuf, sz);
    c1 = caps.dump(abuf, sizeof(abuf));
    c2 = ncaps.dump(bbuf, sizeof(bbuf));
    EXPECT_EQ(c1, c2);
    EXPECT_EQ(strcmp(abuf, bbuf), 0);
    EXPECT_THROW(ncaps.dump(cbuf, sizeof(cbuf)), out_of_range);
    EXPECT_THROW(ncaps.dump(nullptr, 1), invalid_argument);
  } catch (exception& e) {
    FAIL() << e.what();
  }
}

static void checkTypeByIndex(const Caps& caps) {
  EXPECT_EQ(caps.size(), 9);
  EXPECT_EQ(caps[0].type(), CAPS_MEMBER_TYPE_INT32);
  EXPECT_EQ(caps[1].type(), CAPS_MEMBER_TYPE_UINT32);
  EXPECT_EQ(caps[2].type(), CAPS_MEMBER_TYPE_STRING);
  EXPECT_EQ(caps[3].type(), CAPS_MEMBER_TYPE_STRING);
  EXPECT_EQ(caps[4].type(), CAPS_MEMBER_TYPE_FLOAT);
  EXPECT_EQ(caps[5].type(), CAPS_MEMBER_TYPE_INT64);
  EXPECT_EQ(caps[6].type(), CAPS_MEMBER_TYPE_DOUBLE);
  EXPECT_EQ(caps[7].type(), CAPS_MEMBER_TYPE_BINARY);
  EXPECT_EQ(caps[8].type(), CAPS_MEMBER_TYPE_OBJECT);
  Caps sub = caps[8];
  EXPECT_EQ(sub.size(), 1);
  EXPECT_EQ(sub[0].type(), CAPS_MEMBER_TYPE_VOID);
}

static void checkTypeByIterator(const Caps& caps) {
  auto it = caps.iterate();
  EXPECT_EQ(it.next().type(), CAPS_MEMBER_TYPE_INT32);
  EXPECT_EQ(it.next().type(), CAPS_MEMBER_TYPE_UINT32);
  EXPECT_EQ(it.next().type(), CAPS_MEMBER_TYPE_STRING);
  EXPECT_EQ(it.next().type(), CAPS_MEMBER_TYPE_STRING);
  EXPECT_EQ(it.next().type(), CAPS_MEMBER_TYPE_FLOAT);
  EXPECT_EQ(it.next().type(), CAPS_MEMBER_TYPE_INT64);
  EXPECT_EQ(it.next().type(), CAPS_MEMBER_TYPE_DOUBLE);
  EXPECT_EQ(it.next().type(), CAPS_MEMBER_TYPE_BINARY);
  auto v = it.next();
  EXPECT_EQ(v.type(), CAPS_MEMBER_TYPE_OBJECT);
  EXPECT_EQ(it.hasNext(), false);
  Caps sub = v;
  it = sub.iterate();
  EXPECT_EQ(it.next().type(), CAPS_MEMBER_TYPE_VOID);
  EXPECT_EQ(it.hasNext(), false);
}

TEST(TestCaps, checkType) {
  Caps caps, ncaps;
  writeCaps(caps);
  char buf[256];

  try {
    auto sz = caps.serialize(buf, sizeof(buf));
    ncaps.parse(buf, sz);
    checkTypeByIndex(caps);
    checkTypeByIndex(ncaps);
    checkTypeByIterator(caps);
    checkTypeByIterator(ncaps);
  } catch (exception& e) {
    FAIL() << e.what();
  }
}

TEST(TestCaps, copymove) {
  Caps a;
  writeCaps(a);
  Caps b = a;
  iterateCaps(a);
  iterateCaps(b);
  Caps c = move(a);
  iterateCaps(c);
  EXPECT_EQ(a.empty(), true);
  a = move(b);
  iterateCaps(a);
  EXPECT_EQ(b.empty(), true);
  b = c;
  iterateCaps(b);
  iterateCaps(c);
  Caps d{move(a)};
  iterateCaps(d);
  EXPECT_EQ(a.empty(), true);
}

TEST(TestCaps, initializer_list) {
  Caps a{ 1, true, "hello", string("world"), (float)0.1, (int64_t)10000LL, (double)1.1 };
  a << vector<char>{ 'f', 'o', 'o' };
  a << Caps{{}};
  readCaps(a);
  Caps c{ { "hello", "world", 233 } };
  EXPECT_EQ(c.size(), 1);
  Caps d = c[0];
  auto it = d.iterate();
  string s;
  it >> s;
  EXPECT_EQ(s, "hello");
  it >> s;
  EXPECT_EQ(s, "world");
  EXPECT_EQ((int32_t)it.next(), 233);
  EXPECT_EQ(it.hasNext(), false);
}
