#include <cstdarg>
#include <cstdio>
#include <cstdlib>

namespace nw4r::db {

// The Wii implementation stops the processor after printing a diagnostic.
// Keep that fail-fast contract on the PC host without emulating PPC stack
// walking or the Wii-only VI/debug-console hardware.
[[noreturn]] void Panic(const char* file, int line, const char* format, ...) {
    std::fprintf(stderr, "NW4R panic at %s:%d: ", file ? file : "<unknown>", line);

    va_list arguments;
    va_start(arguments, format);
    if (format) {
        std::vfprintf(stderr, format, arguments);
    } else {
        std::fputs("<no message>", stderr);
    }
    va_end(arguments);

    std::fputc('\n', stderr);
    std::fflush(stderr);
    std::abort();
}

}  // namespace nw4r::db
