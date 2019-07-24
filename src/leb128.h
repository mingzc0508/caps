#pragma once

#include <stdexcept>

#define LEB128_BYTE_MASK 0x7f
#define LEB128_BITS_PER_BYTE 7
#define LEB128_MAX_INT32_BYTES 5
#define LEB128_MAX_INT64_BYTES 10

namespace rokid {

template <typename T, typename R, int32_t M = std::is_same<R, int32_t>::value ? LEB128_MAX_INT32_BYTES : LEB128_MAX_INT64_BYTES,
         typename std::enable_if<std::is_same<R, int32_t>::value || std::is_same<R, int64_t>::value, R>::type* = nullptr,
         typename std::enable_if<std::is_same<T, const uint8_t>::value || std::is_same<T, uint8_t>::value, T>::type* = nullptr>
T* leb128Read(T* in, R& res) {
  R cur;
  T* p = in;
  uint8_t curShift{0};
  uint8_t signExtBits{std::is_same<R, int32_t>::value ? 32 : 64};

  res = 0;
  while (true) {
    if (p - in >= M)
      throw std::length_error("input data corrupted");
    cur = *p;
    res |= (cur & LEB128_BYTE_MASK) << curShift;
    curShift += LEB128_BITS_PER_BYTE;
    ++p;
    if (cur <= LEB128_BYTE_MASK)
      break;
  }
  if (curShift < (sizeof(R) << 3))
    signExtBits -= curShift;
  res = (res << signExtBits) >> signExtBits;
  return p;
}

template <typename T, typename R, int32_t M = std::is_same<R, uint32_t>::value ? LEB128_MAX_INT32_BYTES : LEB128_MAX_INT64_BYTES,
         typename std::enable_if<std::is_same<R, uint32_t>::value || std::is_same<R, uint64_t>::value, R>::type* = nullptr,
         typename std::enable_if<std::is_same<T, const uint8_t>::value || std::is_same<T, uint8_t>::value, T>::type* = nullptr>
T* uleb128Read(T* in, R& res) {
  R cur;
  T* p = in;
  uint8_t curShift{0};

  res = 0;
  while (true) {
    if (p - in >= M)
      throw std::length_error("input data corrupted");
    cur = *p;
    res |= (cur & LEB128_BYTE_MASK) << curShift;
    curShift += LEB128_BITS_PER_BYTE;
    ++p;
    if (cur <= LEB128_BYTE_MASK)
      break;
  }
  return p;
}

template <typename T,
         typename std::enable_if<std::is_same<T, int32_t>::value || std::is_same<T, int64_t>::value, T>::type* = nullptr>
uint8_t* leb128Write(T v, uint8_t* out, uint32_t size) {
  bool more{true};
  int32_t cur;
  auto p = out;
  while (more) {
    if (p - out >= size)
      throw std::out_of_range("no enough buffer");
    cur = v & LEB128_BYTE_MASK;
    v >>= 7;
    if ((v == 0 && cur < 0x40) || (v == -1 && cur >= 0x40)) {
      more = false;
    } else {
      cur |= 0x80;
    }
    *p = cur;
    ++p;
  }
  return p;
}

template <typename T,
         typename std::enable_if<std::is_same<T, uint32_t>::value || std::is_same<T, uint64_t>::value, T>::type* = nullptr>
uint8_t* uleb128Write(T v, uint8_t* out, uint32_t size) {
  bool more{true};
  uint32_t cur;
  auto p = out;
  while (more) {
    if (p - out >= size)
      throw std::out_of_range("no enough buffer");
    cur = v & LEB128_BYTE_MASK;
    v >>= 7;
    if (v == 0) {
      more = false;
    } else {
      cur |= 0x80;
    }
    *p = cur;
    ++p;
  }
  return p;
}

} // namespace rokid
