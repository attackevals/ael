/**
 * Standalone C++ implementation of base64 encoding, based on code from
 * https://web.mit.edu/freebsd/head/contrib/wpa/src/utils/base64.c
 * Modified to be position-independent
 *
 * Reference:
 *      https://web.mit.edu/freebsd/head/contrib/wpa/src/utils/base64.c
 *      License and copyright info: https://web.mit.edu/freebsd/head/contrib/wpa/
*/

#pragma once

int PI_Base64Encode(const unsigned char* src, size_t src_len, unsigned char* dst_buf, size_t* dst_buf_len);
