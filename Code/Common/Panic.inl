// Author: Jake Rieger
// Created: 1/7/2025.
//

#pragma once

#include <cstdarg>
#include <cstdio>
#include "Types.hpp"

namespace x {
    [[noreturn]] static void __panic_impl(cstr file, i32 line, cstr func, cstr fmt, ...) noexcept {
        va_list args;
        va_start(args, fmt);
        char message[1024];
        vsnprintf(message, sizeof(message), fmt, args);
        va_end(args);
        printf("%s:%d :: PANIC\n -- In function: `%s`\n -- Error: %s\n", file, line, func, message);
        std::abort();
    }

#ifndef PANIC
    #define Panic(fmt, ...) x::__panic_impl(__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#endif
}  // namespace x