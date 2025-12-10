#pragma once

#include <stdint.h>

constexpr uint32_t val_32_const = 0x811c9dc5;
constexpr uint32_t prime_32_const = 0x1000193;

inline constexpr uint32_t hash_32_fnv1a_const(const char* const str, const uint32_t value = val_32_const) noexcept {
	return (str[0] == '\0') ? value : hash_32_fnv1a_const(&str[1], (value ^ uint32_t(str[0])) * prime_32_const);
}

constexpr uint32_t cx_fnv_hash(const char* str) noexcept  {
	return hash_32_fnv1a_const(str);
}

consteval auto by_fnv1a(auto name) noexcept {
    auto hash = cx_fnv_hash(name);
    return [=](const auto& text) constexpr -> bool {
        return hash == cx_fnv_hash(text);
    };
}
