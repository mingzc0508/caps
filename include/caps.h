#pragma once

#include <stdint.h>

#define CAPS_VERSION 5

#define CAPS_MEMBER_TYPE_INT32 'i'
#define CAPS_MEMBER_TYPE_UINT32 'u'
#define CAPS_MEMBER_TYPE_INT64 'l'
#define CAPS_MEMBER_TYPE_UINT64 'k'
#define CAPS_MEMBER_TYPE_FLOAT 'f'
#define CAPS_MEMBER_TYPE_DOUBLE 'd'
#define CAPS_MEMBER_TYPE_STRING 'S'
#define CAPS_MEMBER_TYPE_BINARY 'B'
#define CAPS_MEMBER_TYPE_OBJECT 'O'
#define CAPS_MEMBER_TYPE_VOID 'V'

#ifdef __cplusplus
#include <memory>
#include <string>
#include <vector>
#include <initializer_list>

namespace rokid {

class Member;
typedef std::shared_ptr<Member> MemberPointer;

class Caps {
private:
  /// \brief Caps中成员变量数据封装类
  class Value {
  public:
    Value(bool v);
    Value(int8_t v);
    Value(uint8_t v);
    Value(int16_t v);
    Value(uint16_t v);
    Value(int32_t v);
    Value(uint32_t v);
    Value(int64_t v);
    Value(uint64_t v);
    Value(float v);
    Value(double v);
    Value(const char* v);
    Value(const std::string& v);
    Value(std::initializer_list<Value> list);
    Value();

    operator bool() const;
    operator int8_t() const;
    operator uint8_t() const;
    operator int16_t() const;
    operator uint16_t() const;
    operator int32_t() const;
    operator uint32_t() const;
    operator int64_t() const;
    operator uint64_t() const;
    operator float() const;
    operator double() const;
    operator const std::string&() const;
    operator Caps() const;
    void get(std::vector<char>& out) const;

    /// \return 数据类型 (CAPS_MEMBER_TYPE_INT32 etc.)
    char type() const;

    inline bool isVoid() const { return type() == CAPS_MEMBER_TYPE_VOID; }

  private:
    Value(MemberPointer m);

    MemberPointer member;

    friend class Caps;
  };

public:
  Caps();
  Caps(std::initializer_list<Value> list);
  ~Caps();

  /// \brief copy constructor
  Caps(const Caps& o);
  /// \brief move constructor
  Caps(Caps&& o);
  /// \brief copy assignment
  Caps& operator = (const Caps& o);
  /// \brief move assignment
  Caps& operator = (Caps&& o);

  /// \brief 序列化
  /// \param out 序列化结果输出buffer
  /// \param size buffer size
  /// \throws invalid_argument
  /// \throws out_of_range
  uint32_t serialize(void* out, uint32_t size) const;

  /// \brief 写入void类型
  void write();
  /// \brief 写入bool类型
  inline void write(bool v) { write((uint32_t)(v ? 1 : 0)); }
  /// \brief 写入signed char类型
  inline void write(int8_t v) { write((int32_t)v); }
  /// \brief 写入unsigned char类型
  inline void write(uint8_t v) { write((uint32_t)v); }
  /// \brief 写入signed short类型
  inline void write(int16_t v) { write((int32_t)v); }
  /// \brief 写入unsigned short类型
  inline void write(uint16_t v) { write((uint32_t)v); }
  /// \brief 写入带符号32位整数类型
  void write(int32_t v);
  /// \brief 写入不带符号32位整数类型
  void write(uint32_t v);
  /// \brief 写入带符号64位整数类型
  void write(int64_t v);
  /// \brief 写入不带符号64位整数类型
  void write(uint64_t v);
  /// \brief 写入32位浮点数类型
  void write(float v);
  /// \brief 写入64位浮点数类型
  void write(double v);
  /// \brief 写入字符串类型
  void write(const char* v);
  /// \brief 写入字符串类型
  inline void write(const std::string& v) { write(v.c_str()); }
  /// \brief 写入二进制数据类型
  void write(const void* data, uint32_t size);
  /// \brief 写入二进制数据类型
  inline void write(const std::vector<char>& v) { write(v.data(), v.size()); }
  /// \brief 写入Caps类型
  void write(const Caps& v);
  inline void operator << (bool v) { write(v); }
  inline void operator << (int8_t v) { write(v); }
  inline void operator << (uint8_t v) { write(v); }
  inline void operator << (int16_t v) { write(v); }
  inline void operator << (uint16_t v) { write(v); }
  inline void operator << (int32_t v) { write(v); }
  inline void operator << (uint32_t v) { write(v); }
  inline void operator << (float v) { write(v); }
  inline void operator << (int64_t v) { write(v); }
  inline void operator << (uint64_t v) { write(v); }
  inline void operator << (double v) { write(v); }
  inline void operator << (const char* v) { write(v); }
  inline void operator << (const std::string& v) { write(v); }
  inline void operator << (const std::vector<char>& v) { write(v); }
  inline void operator << (const Caps& v) { write(v); }

  /// \brief 从二进制数据反序列化生成Caps
  ///        Caps原来的数据将会被清除
  /// \param in 输入二进制数据指针
  /// \param size 输入的二进制数据大小
  /// \throws invalid_argument in == nullptr或size长度不正确
  /// \throws domain_error 输入二进制数据不是Caps序列化生成的，格式错误
  void parse(const void* in, uint32_t size);

  /// \brief 按下标访问Caps内数据成员
  Value at(uint32_t i) const;
  /// \brief 按下标访问Caps内数据成员
  inline Value operator[](uint32_t i) const { return at(i); }

  class type_error : public std::runtime_error {
  public:
    explicit type_error(const std::string& msg) : std::runtime_error(msg) {}
    explicit type_error(const char* msg) : std::runtime_error(msg) {}
  };

  /// \brief Caps内数据成员迭代器
  class iterator {
  public:
    bool hasNext() const;
    Value next() const;

    inline void operator >> (bool& v) const { v = next(); }
    inline void operator >> (int8_t& v) const { v = next(); }
    inline void operator >> (uint8_t& v) const { v = next(); }
    inline void operator >> (int16_t& v) const { v = next(); }
    inline void operator >> (uint16_t& v) const { v = next(); }
    inline void operator >> (int32_t& v) const { v = next(); }
    inline void operator >> (uint32_t& v) const { v = next(); }
    inline void operator >> (int64_t& v) const { v = next(); }
    inline void operator >> (uint64_t& v) const { v = next(); }
    inline void operator >> (float& v) const { v = next(); }
    inline void operator >> (double& v) const { v = next(); }
    inline void operator >> (std::string& v) const { v.assign(next()); }
    inline void operator >> (std::vector<char>& v) const { next().get(v); }
    inline void operator >> (Caps& v) const { v = next(); }

  private:
    const std::vector<MemberPointer>* members;
    std::weak_ptr<int32_t> aliveIndicator;
    mutable uint32_t index;

    friend class Caps;
  };
  /// \return 迭代器
  iterator iterate(uint32_t idx = 0) const;

  bool empty() const;

  /// \return Caps内数据成员数量
  uint32_t size() const;

  /// \brief 生成Caps内部数据可视字符串
  /// \param out 输出缓冲区
  /// \param size 缓冲区大小
  /// \return 字符串长度
  uint32_t dump(char* out, uint32_t size) const;

private:
  uint8_t* serializeMemberDesc(uint8_t* out, uint32_t size) const;

  uint8_t* serializeMembers(uint8_t* out, uint32_t size) const;

  void serializeHeader(uint8_t* out, uint32_t size) const;

  void clearMembers();

  const uint8_t* parseHeader(const uint8_t* p, uint32_t& totalSize);

  void parseMembers(const uint8_t* in, uint32_t psize,
      const uint8_t* desc, uint32_t descLen);

  uint32_t dump(uint32_t indent, char* out, uint32_t size) const;

private:
  std::vector<MemberPointer> members;
  std::shared_ptr<int32_t> aliveIndicator;
};

} // namespace rokid

extern "C" {
#endif // __cplusplus

/**
typedef intptr_t caps_t;

// create创建的caps_t对象可写，不可读
caps_t caps_create();

// parse创建的caps_t对象可读，不可写
int32_t caps_parse(const void* data, uint32_t length, caps_t* result);

// 如果serialize生成的数据长度大于'bufsize'，将返回所需的buf size，
// 但'buf'不会写入任何数据，需外部重新分配更大的buf，再次调用serialize
int32_t caps_serialize(caps_t caps, void* buf, uint32_t bufsize);

int32_t caps_write_integer(caps_t caps, int32_t v);

int32_t caps_write_long(caps_t caps, int64_t v);

int32_t caps_write_float(caps_t caps, float v);

int32_t caps_write_double(caps_t caps, double v);

int32_t caps_write_string(caps_t caps, const char* v);

int32_t caps_write_binary(caps_t caps, const void* data, uint32_t length);

int32_t caps_write_object(caps_t caps, caps_t v);

int32_t caps_write_void(caps_t caps);

int32_t caps_read_integer(caps_t caps, int32_t* r);

int32_t caps_read_long(caps_t caps, int64_t* r);

int32_t caps_read_float(caps_t caps, float* r);

int32_t caps_read_double(caps_t caps, double* r);

int32_t caps_read_string(caps_t caps, const char** r);

int32_t caps_read_binary(caps_t caps, const void** r, uint32_t* length);

int32_t caps_read_object(caps_t caps, caps_t* r);

int32_t caps_read_void(caps_t caps);

void caps_destroy(caps_t caps);

// 'data' 必须不少于8字节
// 根据8字节信息，得到整个二进制数据长度及caps版本
int32_t caps_binary_info(const void* data, uint32_t* version, uint32_t* length);
*/

#ifdef __cplusplus
} // extern "C"
#endif
