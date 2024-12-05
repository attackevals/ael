#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>

typedef unsigned char BYTE;

static inline bool is_base64(BYTE c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

std::string base64_encode_string(const std::string data) {
    static constexpr char sEncodingTable[] = {
      'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
      'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
      'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
      'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
      'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
      'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
      'w', 'x', 'y', 'z', '0', '1', '2', '3',
      '4', '5', '6', '7', '8', '9', '+', '/'
    };

    size_t in_len = data.size();
    size_t out_len = 4 * ((in_len + 2) / 3);
    std::string ret(out_len, '\0');
    size_t i;
    char* p = const_cast<char*>(ret.c_str());

    for (i = 0; i < in_len - 2; i += 3) {
        *p++ = sEncodingTable[(data[i] >> 2) & 0x3F];
        *p++ = sEncodingTable[((data[i] & 0x3) << 4) | ((int)(data[i + 1] & 0xF0) >> 4)];
        *p++ = sEncodingTable[((data[i + 1] & 0xF) << 2) | ((int)(data[i + 2] & 0xC0) >> 6)];
        *p++ = sEncodingTable[data[i + 2] & 0x3F];
    }
    if (i < in_len) {
        *p++ = sEncodingTable[(data[i] >> 2) & 0x3F];
        if (i == (in_len - 1)) {
            *p++ = sEncodingTable[((data[i] & 0x3) << 4)];
            *p++ = '=';
        }
        else {
            *p++ = sEncodingTable[((data[i] & 0x3) << 4) | ((int)(data[i + 1] & 0xF0) >> 4)];
            *p++ = sEncodingTable[((data[i + 1] & 0xF) << 2)];
        }
        *p++ = '=';
    }

    return ret;
}

std::vector<BYTE> base64_decode(std::string const& encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    BYTE char_array_4[4], char_array_3[3];
    std::vector<BYTE> ret;

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret.push_back(char_array_3[i]);
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
    }

    return ret;
}
// XOR key for config
std::vector<unsigned char> key = base64_decode(CONFIG_ENCRYPTION_KEY_BASE64);

std::string XorStringData(std::string data)
{
    std::string output = data;
    for (size_t i = 0; i < data.size(); ++i)
    {
        output[i] = data[i] ^ key[i % key.size()];
    }

    return output;
}

std::string ReadConfig(std::string filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        printf("Failed to open config file.\n");
        exit(-1);
    }

    std::string content((std::istreambuf_iterator<char>(file)),
        (std::istreambuf_iterator<char>()));

    file.close();

    return content;
}

void WriteEncryptedConfig(std::string filePath, std::string content) {
    std::ofstream write(filePath);
    write << content;
    write.close();
    return;
}

int main(int argc, char* argv[]) {
    std::cout << argc << std::endl;
    if (argc >= 2) {
        std::string configPath(argv[1]);
        printf("Reading in config from: %s\n", argv[1]);

        std::string plainText = ReadConfig(configPath);
        std::cout << "Config data is: " << plainText << std::endl;

        std::string xorData = XorStringData(plainText);
        std::cout << "XOR'd config string." << std::endl;

        std::string base64Encoded = base64_encode_string(xorData);
        std::cout << "Base64 encdoed string contents: " << base64Encoded << std::endl;

        WriteEncryptedConfig(configPath + ".enc", base64Encoded);
        std::cout << "Wrote config to " << configPath << ".enc" << std::endl;
        return 0;
    }

    std::cout << "Missing required config path." << std::endl;
    return -1;
}