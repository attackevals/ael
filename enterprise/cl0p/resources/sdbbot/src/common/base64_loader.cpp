#include <base64_loader.hpp>
#include <Windows.h>

namespace common {
	const static std::byte base64_table[64] =
	{std::byte('A'),std::byte('B'),std::byte('C'),std::byte('D'),std::byte('E'),std::byte('F'),std::byte('G'),std::byte('H'),
	std::byte('I'),std::byte('J'),std::byte('K'),std::byte('L'),std::byte('M'),std::byte('N'),std::byte('O'),std::byte('P'),
	std::byte('Q'),std::byte('R'),std::byte('S'),std::byte('T'),std::byte('U'),std::byte('V'),std::byte('W'),std::byte('X'),
	std::byte('Y'),std::byte('Z'),std::byte('a'),std::byte('b'),std::byte('c'),std::byte('d'),std::byte('e'),std::byte('f'),
	std::byte('g'),std::byte('h'),std::byte('i'),std::byte('j'),std::byte('k'),std::byte('l'),std::byte('m'),std::byte('n'),
	std::byte('o'),std::byte('p'),std::byte('q'),std::byte('r'),std::byte('s'),std::byte('t'),std::byte('u'),std::byte('v'),
	std::byte('w'),std::byte('x'),std::byte('y'),std::byte('z'),std::byte('0'),std::byte('1'),std::byte('2'),std::byte('3'),
	std::byte('4'),std::byte('5'),std::byte('6'),std::byte('7'),std::byte('8'),std::byte('9'),std::byte('+'),std::byte('/')};

	std::span<std::byte> base64_encode(std::span<std::byte> source)
	{
		// calculate output length, use uint8_t so arithmetic operations work
		size_t in_len = source.size();
		size_t out_len = 4 * ((in_len + 2) / 3);
		std::byte* out = static_cast<std::byte*>(VirtualAlloc(NULL, out_len, MEM_COMMIT, PAGE_READWRITE));
		uint8_t* start = reinterpret_cast<uint8_t*>(source.data());
		int i = 0; // block iterator
		int j = 0; // padding iterator
		int z = 0; // char in out iterator
		
		uint8_t buf_3[3]; // contains blocks of 3 from source chars
		uint8_t buf_4[4]; // contains blocks of 4 for output chars

		while (in_len--) {
			buf_3[i++] = *(start++);
			if (i == 3) {
				buf_4[0] = (buf_3[0] & 0xfc) >> 2;
				buf_4[1] = ((buf_3[0] & 0x03) << 4) + ((buf_3[1] & 0xf0) >> 4);
				buf_4[2] = ((buf_3[1] & 0x0f) << 2) + ((buf_3[2] & 0xc0) >> 6);
				buf_4[3] = buf_3[2] & 0x3f;

				for (i = 0; (i < 4); i++) {
					out[z] = base64_table[buf_4[i]];
					z++;
				}

				i = 0;
			}
		}

		// padding
		if (i)
		{
			for (j = i; j < 3; j++)
				buf_3[j] = '\0';

			buf_4[0] = (buf_3[0] & 0xfc) >> 2;
			buf_4[1] = ((buf_3[0] & 0x03) << 4) + ((buf_3[1] & 0xf0) >> 4);
			buf_4[2] = ((buf_3[1] & 0x0f) << 2) + ((buf_3[2] & 0xc0) >> 6);
			buf_4[3] = buf_3[2] & 0x3f;

			for (j = 0; (j < i + 1); j++) {
				out[z] = base64_table[buf_4[j]];
				z++;
			}

			while ((i++ < 3)) {
				out[z] = std::byte('=');
				z++;
			}
		}

		return std::span<std::byte>(out, out_len);
	}
}