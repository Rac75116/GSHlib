#pragma once
#include "TypeDef.hpp"
namespace gsh {
class Exception {
  char str[512];
  char* cur = str;
  void write(const char* x) {
    for(int i = 0; i != 512; ++i, ++cur) {
      if(x[i] == '\0') break;
      *cur = x[i];
    }
  }
  void write(long long x) {
    if(x == 0) *(cur++) = '0';
    else {
      if(x < 0) {
        *(cur++) = '-';
        x = -x;
      }
      char buf[20];
      int i = 0;
      while(x != 0) buf[i++] = x % 10 + '0', x /= 10;
      while(i--) *(cur++) = buf[i];
    }
  }
  template<class T, class... Args>
  void generate_message(T x, Args... args) {
    write(x);
    if constexpr(sizeof...(Args) > 0) generate_message(args...);
  }
public:
  Exception() noexcept { *cur = '\0'; }
  Exception(const Exception& x) noexcept {
    for(int i = 0; i != 512; ++i) str[i] = x.str[i];
    cur = x.cur;
  }
  explicit Exception(const char* what_arg) noexcept {
    for(int i = 0; i != 512; ++i, ++cur) {
      *cur = what_arg[i];
      if(what_arg[i] == '\0') break;
    }
  }
  template<class... Args>
  explicit Exception(Args... args) noexcept {
    generate_message(args...);
    *cur = '\0';
  }
  Exception& operator=(const Exception& x) noexcept {
    for(int i = 0; i != 512; ++i) str[i] = x.str[i];
    cur = x.cur;
    return *this;
  }
  const char* what() const noexcept { return str; }
};
} // namespace gsh
