#include <cstdio> // FILE, fread, fwrite, fflush
#if __has_include(<unistd.h>) && __has_include(<fcntl.h>)
#include "TypeDef.hpp"
#include "Util.hpp"
#include <fcntl.h>
#include <unistd.h> // read, write
namespace gsh {
enum class OpenMode { ReadOnly = O_RDONLY,
  WriteOnly = O_WRONLY,
  ReadWrite = O_RDWR,
  Append = O_APPEND,
  Create = O_CREAT,
  Trunc = O_TRUNC,
  Binary = O_BINARY };
constexpr OpenMode operator|(OpenMode a, OpenMode b) noexcept {
  return static_cast<OpenMode>(static_cast<i32>(a) | static_cast<i32>(b));
}
class FileDescriptor {
  i32 fd;
  OpenMode mode;
public:
  FILE* file_ptr() && {
    i32 copy = fd;
    fd = 0;
    const c8* format;
    if(mode == OpenMode::ReadOnly) {
      format = "r";
    } else if(mode == OpenMode::WriteOnly) {
      format = "w";
    } else if(mode == (OpenMode::WriteOnly | OpenMode::Append)) {
      format = "a";
    } else if(mode == OpenMode::ReadWrite) {
      format = "r+";
    } else if(mode == (OpenMode::ReadWrite | OpenMode::Append)) {
      format = "a+";
    } else if(mode == (OpenMode::ReadOnly | OpenMode::Binary)) {
      format = "rb";
    } else if(mode == (OpenMode::WriteOnly | OpenMode::Binary)) {
      format = "wb";
    } else {
      Unreachable();
    }
    return fdopen(copy, format);
  }
};
} // namespace gsh
#else
#endif
