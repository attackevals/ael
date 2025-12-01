/*
 * obfuscation.hpp:
 *      About:
 *          XOR-encrypts strings at compile-time and decrypts them at run-time.
 *          Modified from: https://github.com/Tai7sy/vs-obfuscation/blob/2092b064f2d204e16e794448d677e4471e43314b/obfuscation.h
 *      MITRE ATT&CK Techniques:
 *          T1027: Obfuscated Files or Information
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html
 */

#pragma once

#include <stdint.h>

template <int X> struct EnsureCompileTime {
	enum : int {
		Value = X
	};
};

//Use Compile-Time as seed
#define OBF_RAND_SEED ((__TIME__[7] - '0') * 1  + (__TIME__[6] - '0') * 10  + \
                  (__TIME__[4] - '0') * 60   + (__TIME__[3] - '0') * 600 + \
                  (__TIME__[1] - '0') * 3600 + (__TIME__[0] - '0') * 36000)

constexpr int LinearCongruentGenerator(int Rounds) {
	return 1013904223 + 1664525 * ((Rounds> 0) ? LinearCongruentGenerator(Rounds - 1) : OBF_RAND_SEED & 0xFFFFFFFF);
}
#define OBF_RANDOM() EnsureCompileTime<LinearCongruentGenerator(10)>::Value //10 Rounds
#define OBF_RANDOM_NUMBER(Min, Max) (Min + (OBF_RANDOM() % (Max - Min + 1)))

template <int... Pack> struct IndexList {};

template <typename IndexList, int Right> struct Append;
template <int... Left, int Right> struct Append<IndexList<Left...>, Right> {
	typedef IndexList<Left..., Right> Result;
};

template <int N> struct ConstructIndexList {
	typedef typename Append<typename ConstructIndexList<N - 1>::Result, N - 1>::Result Result;
};
template <> struct ConstructIndexList<0> {
	typedef IndexList<> Result;
};

const char OBF_XORKEY = static_cast<char>(OBF_RANDOM_NUMBER(0, 0xFF));
__forceinline constexpr char EncryptCharacter(const char Character, int Index) {
	return Character ^ ((OBF_XORKEY + Index) % 256);
}
template <typename IndexList> class CXorString;
template <int... Index> class CXorString<IndexList<Index...> > {
private:
	char Value[sizeof...(Index)+1];
public:
	__forceinline constexpr CXorString(const char* const String)
		: Value{ EncryptCharacter(String[Index], Index)... } {}

	__forceinline char* decrypt() {
		for (int t = 0; t < sizeof...(Index); t++) {
			Value[t] = Value[t] ^ ((OBF_XORKEY + t) % 256);
		}
		Value[sizeof...(Index)] = '\0';
		return Value;
	}

	__forceinline char* get() {
		return Value;
	}
};

const wchar_t OBF_XORKEYW = static_cast<wchar_t>(OBF_RANDOM_NUMBER(0, 0xFFFF));
__forceinline constexpr wchar_t EncryptCharacterW(const wchar_t Character, int Index) {
	return Character ^ ((OBF_XORKEYW + Index) % 65536);
}
template <typename IndexList> class CXorStringW;
template <int... Index> class CXorStringW<IndexList<Index...> > {
private:
	wchar_t Value[sizeof...(Index)+1];
public:
	__forceinline constexpr CXorStringW(const wchar_t* const String)
		: Value{ EncryptCharacterW(String[Index], Index)... } {}

	__forceinline wchar_t* decrypt() {
		for (int t = 0; t < sizeof...(Index); t++) {
			Value[t] = Value[t] ^ ((OBF_XORKEYW + t) % 65536);
		}
		Value[sizeof...(Index)] = L'\0';
		return Value;
	}

	__forceinline wchar_t* get() {
		return Value;
	}
};

#define XorString( String ) ( CXorString<ConstructIndexList<sizeof( String ) - 1>::Result>( String ).decrypt() )
#define XorStringW( String ) ( CXorStringW<ConstructIndexList<(sizeof( String ) / sizeof(wchar_t)) - 1>::Result>( String ).decrypt() )
