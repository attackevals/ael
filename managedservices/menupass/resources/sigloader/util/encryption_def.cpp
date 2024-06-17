#include "encryption_def.h"

//Setting up encryption keys

//RC4
std::string rc4_string_key = XorStr(DEFAULT_RC4_KEY);
CryptoPP::SecByteBlock rc4_key(reinterpret_cast<const byte*>(&rc4_string_key[0]), rc4_string_key.size());
DWORD rc4_keyLength = DEFAULT_RC4_KEY_LEN;
//AES
std::string aes_string_key = XorStr(DEFAULT_AES_KEY);
CryptoPP::SecByteBlock aes_key(reinterpret_cast<const byte*>(&aes_string_key[0]), aes_string_key.size());
CryptoPP::byte aes_iv[] = DEFAULT_AES_IV;
//DES
std::string des_string_key = XorStr(DEFAULT_DES_KEY);
CryptoPP::SecByteBlock des_key(reinterpret_cast<const byte*>(&des_string_key[0]), des_string_key.size());
CryptoPP::byte des_iv[] = DEFAULT_DES_IV;
//XOR
unsigned char xor_key = DEFAULT_XOR_KEY;

long int data_size = DEFAULT_DATA_SIZE;

std::string sig = XorStr(DEFAULT_SIGNATURE);
std::vector<unsigned char> signature = std::vector<unsigned char>(sig.data(), sig.data() + sig.length());

int debug = DEFAULT_DEBUG;


void debug_message(std::string message, std::string message_type) {
	std::ofstream outfile;
	outfile.open(XorStr("debug.txt"), std::ios_base::app);

	std::string time = currentDateTime();
	outfile << message_type + " - " + time + " - ";
	outfile << message;
	outfile << "\n";
}

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
std::string currentDateTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
	// for more information about date/time format
	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

	return buf;
}