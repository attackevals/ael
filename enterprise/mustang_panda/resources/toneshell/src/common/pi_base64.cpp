#include "pi_base64.hpp"

/*
 * Performs position-independent base64 encoding.
 * Returns 0 on failure, non-0 value on success.
 * Failure occurs if destination buf is not big enough
 * to hold encoded src.
 * On input, dst_buf_len contains the size of dst_buf, in bytes.
 * On output, dst_buf_len will contain the actual size of the encoded
 * input.
 * Does not add a null byte at the end.
 */
int PI_Base64Encode(const unsigned char* src, size_t src_len, unsigned char* dst_buf, size_t* dst_buf_len) {
        // ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/
	unsigned char base64_table[64];
	for (int i = 0; i < 26; i++) {
		base64_table[i] = 'A' + i;
		base64_table[26 + i] = 'a' + i;
	}
	for (int i = 0; i < 10; i++) {
		base64_table[52 + i] = '0' + i;
	}
	base64_table[62] = '+';
	base64_table[63] = '/';

    unsigned char *pos;
	const unsigned char *end, *in;
    size_t encoded_len;
	if (dst_buf == nullptr) {
		return 0;
	}
    if (dst_buf_len == nullptr || *dst_buf_len == 0) {
        return 0;
    }
    if (src_len == 0) {
        *dst_buf_len = 0;
        return 1;
    }
    encoded_len = (src_len / 3) * 4;
    if (src_len % 3 != 0) {
        encoded_len += 4; // include padding at end if needed
    }
    if (encoded_len > *dst_buf_len) {
        return 0;
    }

	end = src + src_len;
	in = src;
	pos = dst_buf;
	while (end - in >= 3) {
		*pos++ = base64_table[in[0] >> 2];
		*pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
		*pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
		*pos++ = base64_table[in[2] & 0x3f];
		in += 3;
	}

	if (end - in) {
		*pos++ = base64_table[in[0] >> 2];
		if (end - in == 1) {
			*pos++ = base64_table[(in[0] & 0x03) << 4];
			*pos++ = '=';
		} else {
			*pos++ = base64_table[((in[0] & 0x03) << 4) |
					      (in[1] >> 4)];
			*pos++ = base64_table[(in[1] & 0x0f) << 2];
		}
		*pos++ = '=';
	}
	*dst_buf_len = pos - dst_buf;

	return 1;
}
