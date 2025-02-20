#include <cstdio>  // FILE, fread, fwrite, fflush
#if __has_include(<unistd.h>) && __has_include(<fcntl.h>)
#include <unistd.h>  // read, write
#include <fcntl.h>

#include "TypeDef.hpp"  // gsh::itype
#include "Util.hpp"     // gsh::Unreachable

namespace gsh {

enum class OpenMode { ReadOnly = O_RDONLY, WriteOnly = O_WRONLY, ReadWrite = O_RDWR, Append = O_APPEND, Create = O_CREAT, Trunc = O_TRUNC, Binary = O_BINARY };
constexpr OpenMode operator|(OpenMode a, OpenMode b) noexcept {
    return static_cast<OpenMode>(static_cast<itype::i32>(a) | static_cast<itype::i32>(b));
}

class FileDescriptor {
    itype::i32 fd;
    OpenMode mode;
public:
    FILE* file_ptr() && {
        itype::i32 copy = fd;
        fd = 0;
        const ctype::c8* format;
        switch (mode) {
        case OpenMode::ReadOnly : format = "r"; break;
        case OpenMode::WriteOnly : format = "w"; break;
        case OpenMode::WriteOnly | OpenMode::Append : format = "a"; break;
        case OpenMode::ReadWrite : format = "r+"; break;
        case OpenMode::ReadWrite | OpenMode::Append : format = "a+"; break;
        case OpenMode::ReadOnly | OpenMode::Binary : format = "rb"; break;
        case OpenMode::WriteOnly | OpenMode::Binary : format = "wb"; break;
        default : Unreachable();
        }
        return fdopen(copy, format);
    }
};

}  // namespace gsh

#else
#endif
