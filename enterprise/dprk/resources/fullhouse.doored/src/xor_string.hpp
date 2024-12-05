/**
 * Provides a macro that will obfuscate const char* string literals by XOR-encrypting them
 * with a hardcoded 128-byte key at compile-time and decrypt them at run-time.
 * To use in your C++ code, import the header file and wrap const char* and const wchar_t* string literals
 * with XOR_LIT(str) and XOR_WIDE_LIT(str), and compile with optimization level of 1 or higher.
 * Example: std::cout << XOR_LIT("secret") << std::endl;
 * Example with wide strings: std::wcout << XOR_WIDE_LIT(L"wide secret") << std::endl;
 *
 * References:
 *     https://github.com/andrivet/ADVobfuscator/blob/master/Lib/MetaString.h
 *     https://github.com/Snowapril/String-Obfuscator-In-Compile-Time
*/

#ifndef XOR_OBFUSCATION_MACRO_H_
#define XOR_OBFUSCATION_MACRO_H_
#define XOR_OBFUSCATION_MACRO_KEY_LEN 128

#if defined(_MSC_VER)
#define ALWAYS_INLINE __forceinline
#else
#define ALWAYS_INLINE __attribute__((always_inline))
#endif

#include <array>

namespace xor_obfuscation_macro {
    // 128-byte XOR key
    constexpr std::array<char, XOR_OBFUSCATION_MACRO_KEY_LEN> XorKey = {
        0x65, 0x47, 0x4e, 0x6c, 0x65, 0x6e, 0x4a, 0x72, 0x61, 0x57, 0x4e, 0x70, 0x64, 0x47, 0x74, 0x73,
        0x62, 0x57, 0x31, 0x69, 0x63, 0x47, 0x52, 0x71, 0x59, 0x58, 0x56, 0x35, 0x63, 0x58, 0x5a, 0x30,
        0x61, 0x32, 0x68, 0x6c, 0x65, 0x57, 0x5a, 0x7a, 0x5a, 0x6d, 0x4a, 0x35, 0x65, 0x48, 0x68, 0x78,
        0x61, 0x57, 0x6c, 0x77, 0x65, 0x47, 0x35, 0x32, 0x65, 0x57, 0x64, 0x6f, 0x63, 0x32, 0x31, 0x79,
        0x62, 0x33, 0x68, 0x35, 0x59, 0x33, 0x46, 0x30, 0x62, 0x57, 0x68, 0x32, 0x59, 0x6d, 0x78, 0x30,
        0x65, 0x47, 0x6c, 0x79, 0x59, 0x6e, 0x64, 0x6c, 0x5a, 0x47, 0x70, 0x36, 0x61, 0x58, 0x4a, 0x7a,
        0x62, 0x47, 0x52, 0x33, 0x65, 0x6d, 0x56, 0x6c, 0x59, 0x33, 0x64, 0x79, 0x5a, 0x32, 0x64, 0x74,
        0x65, 0x57, 0x52, 0x32, 0x62, 0x32, 0x31, 0x6f, 0x61, 0x47, 0x78, 0x34, 0x65, 0x6d, 0x39, 0x73
    };

    // Obfuscated string class that is parameterized with a list of Indexes
    template <typename Indexes>
    class ObfuscatedString;

    // Obfuscated wide string class that is parameterized with a list of Indexes
    template <typename Indexes>
    class ObfuscatedWideString;

    template <size_t... I>
    class ObfuscatedString<std::index_sequence<I...>> {
    public:
        // Constructor will get evaluated at compile-time
        // XOR encrypt the string when constructing the ObfuscatedString
        inline constexpr ALWAYS_INLINE ObfuscatedString(const char* str)
            : encrypted_buffer{ xor_bytes(str[I], XorKey[I % XOR_OBFUSCATION_MACRO_KEY_LEN])... } {};

    public:
        // provide runtime decryption routine
        inline const char* decrypt() {
            for (size_t i = 0; i < sizeof...(I); i++) {
                decrypted_buffer[i] = xor_bytes(encrypted_buffer[i], XorKey[i % XOR_OBFUSCATION_MACRO_KEY_LEN]);
            }
            decrypted_buffer[sizeof...(I)] = 0; // null-terminate our string
            return const_cast<const char*>(decrypted_buffer);
        }

    private:
        char decrypted_buffer[sizeof...(I) + 1]{};
        volatile char encrypted_buffer[sizeof...(I)]{}; // this will get populated in the constructor

        // XOR the two bytes an return the result
        inline constexpr char ALWAYS_INLINE xor_bytes(char a, char b) const {
            return a ^ b;
        }
    };

    template <size_t... I>
    class ObfuscatedWideString<std::index_sequence<I...>> {
    public:
        // Constructor will get evaluated at compile-time
        // XOR encrypt the string when constructing the ObfuscatedWideString
        inline constexpr ALWAYS_INLINE ObfuscatedWideString(const wchar_t* str)
            : encrypted_buffer{ xor_bytes(((const char*)str)[I], XorKey[I % XOR_OBFUSCATION_MACRO_KEY_LEN])... } {};

    public:
        // provide runtime decryption routine
        inline const wchar_t* decrypt() {
            for (size_t i = 0; i < sizeof...(I); i++) {
                decrypted_buffer[i] = xor_bytes(encrypted_buffer[i], XorKey[i % XOR_OBFUSCATION_MACRO_KEY_LEN]);
            }
            ((wchar_t*)decrypted_buffer)[sizeof...(I) / sizeof(wchar_t)] = L'\0'; // null-terminate our string
            return (const wchar_t*)decrypted_buffer;
        }

    private:
        char decrypted_buffer[sizeof...(I) + sizeof(wchar_t)]{};
        volatile char encrypted_buffer[sizeof...(I)]{}; // this will get populated in the constructor

        // XOR the two bytes an return the result
        inline constexpr char ALWAYS_INLINE xor_bytes(char a, char b) const {
            return a ^ b;
        }
    };
}

// Replace the given string with its obfuscated counterpart by XOR-encrypting it at compile-time.
// Note: Subtract 1 from size of str to avoid including the null-byte in the integer sequence
// So the string "test" has size 5, and we want the index sequence 0, 1, 2, 3
// std::make_index_sequence<N> produces 0, 1, ... N-1, so we want to pass 1 less than the
// total string size (including the null byte)
#define DEF_XOR_LIT(str) xor_obfuscation_macro::ObfuscatedString<std::make_index_sequence<sizeof(str) - 1>>(str)
#define DEF_XOR_WIDE_LIT(str) xor_obfuscation_macro::ObfuscatedWideString<std::make_index_sequence<sizeof(str) - sizeof(wchar_t)>>(str)

// This macro will replace the const char* string literal with a decryption expression that 
// resolves to a const char*
#define XOR_LIT(str) (DEF_XOR_LIT(str).decrypt())

// This macro will replace the const wchar_t* string literal with a decryption expression that 
// resolves to a const wchar_t*
#define XOR_WIDE_LIT(str) (DEF_XOR_WIDE_LIT(str).decrypt())

#endif