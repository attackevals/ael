/**
 * Standalone C++ implementation of base64 encoding, based on code from
 * https://web.mit.edu/freebsd/head/contrib/wpa/src/utils/base64.c
 *
 * Reference:
 *      https://web.mit.edu/freebsd/head/contrib/wpa/src/utils/base64.c
 *      License and copyright info: https://web.mit.edu/freebsd/head/contrib/wpa/
*/

#pragma once

#include <cstddef>
#include <span>

namespace common {

	std::span<std::byte> base64_encode(std::span<std::byte> src);

}