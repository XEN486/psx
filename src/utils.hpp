#ifndef UTILS_HPP
#define UTILS_HPP

#include "config.hpp"

#include <cstdint>
#include <print>

using u64 = uint64_t;
using i64 = int64_t;
using u32 = uint32_t;
using i32 = int32_t;
using u16 = uint16_t;
using i16 = int16_t;
using u8 = uint8_t;
using i8 = int8_t;

#ifdef ENABLE_LOGGING
#define debug_log(fmt, ...) std::println("{}(): " fmt, __func__, __VA_ARGS__)
#define error_log(fmt, ...) std::println(stderr, "{}(): " fmt, __func__, __VA_ARGS__)
#else
#define debug_log(...)
#define error_log(...)
#endif

#endif