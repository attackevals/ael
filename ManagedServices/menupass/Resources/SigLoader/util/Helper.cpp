#include "Helper.h"
#include "encryption_def.h"


//Encryption order
std::vector<unsigned char>(*encryption_order[])(std::vector<unsigned char>) = DEFAULT_ENCRYPTION_ORDER;



unsigned short ChkSum(unsigned int CheckSum, void *FileBase, int Length)
{

	int *Data;
	int sum;

	if (Length && FileBase != NULL)
	{
		Data = (int *)FileBase;
		do
		{
			sum = *(unsigned short *)Data + CheckSum;
			Data = (int *)((char *)Data + 2);
			CheckSum = (unsigned short)sum + (sum >> 16);
		} while (--Length);
	}

	return CheckSum + (CheckSum >> 16);
}


unsigned int PEChecksum(void *FileBase, unsigned int FileSize)
{

	void *RemainData;
	int RemainDataSize;
	unsigned int PeHeaderSize;
	unsigned int HeaderCheckSum;
	unsigned int PeHeaderCheckSum;
	unsigned int FileCheckSum;
	PIMAGE_NT_HEADERS NtHeaders;
	
	NtHeaders = ImageNtHeader(FileBase);
	if (NtHeaders)
	{
		HeaderCheckSum = NtHeaders->OptionalHeader.CheckSum;
		PeHeaderSize = (unsigned int)NtHeaders - (unsigned int)FileBase +
			((unsigned int)&NtHeaders->OptionalHeader.CheckSum - (unsigned int)NtHeaders);
		RemainDataSize = (FileSize - PeHeaderSize - 4) >> 1;
		RemainData = &NtHeaders->OptionalHeader.Subsystem;
		PeHeaderCheckSum = ChkSum(0, FileBase, PeHeaderSize >> 1);
		FileCheckSum = ChkSum(PeHeaderCheckSum, RemainData, RemainDataSize);

		if (FileSize & 1)
		{
			FileCheckSum += (unsigned short)*((char *)FileBase + FileSize - 1);
		}
	}
	else
	{
		FileCheckSum = 0;
	}

	return (FileSize + FileCheckSum);
}

char* genKey() {

	char _key[16] = "randomkeyrandom";

	srand(time(NULL));
	
	for (int i = 0; i < 15; ++i) {
		_key[i] = '0' + rand() % 72;
	}
	return _key;
}

char* genRandomBytes(size_t length) {
		srand(time(NULL));
		const char* st = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";
		size_t sLen = 26 * 2 + 10 + 7;
		char *rStr;

		rStr = (char*) malloc(sizeof(char) * (length + 1));

		if (!rStr) {
			return (char*)0;
		}

		unsigned int key = 0;

		for (int n = 0; n < length; n++) {
			key = rand() % sLen;
			rStr[n] = st[key];
		}

		rStr[length] = '\0';

		return rStr;
}


void *memcopy(void *const dest, void const *const src, size_t bytes){
	while (bytes-- > (size_t)0)
		((unsigned char *)dest)[bytes] = ((unsigned char const *)src)[bytes];

	return dest;
}

char* getFName(char* _fPath) {

	char *sepd = (strrchr(_fPath, '/') != NULL) ? strrchr(_fPath, '/') : strrchr(_fPath, '\\');

	int l_sep, i = 0;
	char sep = sepd[0];
	if (*_fPath) {
		while (_fPath[i++]) if (_fPath[i] == sep) l_sep = i;
		return _fPath[l_sep] == sep ? &_fPath[l_sep + 1] : _fPath;
	}
	return _fPath;
}

int crypt(unsigned char* data, long dataLen, unsigned char* result) {
	std::vector<unsigned char> data_1(data, data + dataLen);
	if (DEFAULT_RC4_USE) {
		data_1 = rc4(data_1);
	}
	for (int i = 0; i < sizeof(encryption_order) / sizeof(encryption_order[0]); i++) {
		data_1 = encryption_order[i](data_1);
	}
	data = data_1.data();

	for (int i = 0; i < data_1.size(); i++) {
		result[i] = data[i];
	}
	
	

	return data_1.size();
}

/// <summary>
	/// RC4:
	/// 
	/// About:
	///		RC4 encrypt the data using a hardcoded key
	/// Result:
	///		Provided data is encrypted using RC4
	/// Reference: 
	///		https://www.cryptopp.com/wiki/ARC4
	/// </summary>
std::vector<unsigned char> rc4(std::vector<unsigned char> ciphertext) {
	if (debug) {
		debug_message("RC4 - Sigflip", "[DEBUG]");
	}
	std::vector<unsigned char> result(ciphertext.size());

	try {
		CryptoPP::Weak::ARC4::Encryption enc;
		enc.SetKey(rc4_key, rc4_key.size());

		// Perform the decryption
		enc.ProcessData(&result[0], (const CryptoPP::byte*)&ciphertext[0], ciphertext.size());
		std::cout << result.data() << std::endl;

		return result;
	}
	catch (const CryptoPP::Exception& e) {
		std::cerr << e.what() << std::endl;
		exit(1);
	}
}

/// <summary>
/// AES:
/// 
/// About:
///		Calls the AES encryption from cryptopp
/// Result:
///		Provided data is encrypted using AES-CBC
/// </summary>
std::vector<unsigned char> AES(std::vector<unsigned char> plaintext) {
	if (debug) {
		debug_message("AES - Sigflip", "[DEBUG]");
	}
	// Verify provided key size
	if (sizeof(aes_key) != CryptoPP::AES::MAX_KEYLENGTH) {
		throw std::runtime_error("Invalid key size.");
	}


	// Buffer for plaintext
	std::string cipher;
	cipher.reserve((CryptoPP::AES::BLOCKSIZE * 2) + plaintext.size());

	// Decrypt
	CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption AES_encryptor(aes_key, sizeof(aes_key), aes_iv);
	CryptoPP::StringSource(std::string(plaintext.begin(), plaintext.end()), true,
		new CryptoPP::StreamTransformationFilter(AES_encryptor,
			new CryptoPP::StringSink(cipher), CryptoPP::BlockPaddingSchemeDef::BlockPaddingScheme::ZEROS_PADDING
		)
	);
	std::vector<char> _encryptedData = std::vector<char>(cipher.begin(), cipher.end());



	return reinterpret_cast<std::vector<unsigned char>&>(_encryptedData);
}

/// <summary>
/// DES:
/// 
/// About:
///		Calls the DES encryption from cryptopp
/// Result:
///		Provided data is encrypted using DES
/// Reference:
///		https://cryptopp.com/wiki/TripleDES
/// </summary>
std::vector<unsigned char> DES(std::vector<unsigned char> plaintext) {
	if (debug) {
		debug_message("DES - Sigflip", "[DEBUG]");
	}
	// Buffer for plaintext
	std::string cipher;
	cipher.reserve(plaintext.size());


	try {
		CryptoPP::CBC_Mode< CryptoPP::DES_EDE2 >::Encryption DES_Encryptor;

		DES_Encryptor.SetKeyWithIV(des_key, des_key.size(), des_iv);

		CryptoPP::StringSource(std::string(plaintext.begin(), plaintext.end()), true,
			new CryptoPP::StreamTransformationFilter(DES_Encryptor,
				new CryptoPP::StringSink(cipher), CryptoPP::BlockPaddingSchemeDef::BlockPaddingScheme::ZEROS_PADDING
			)
		);
		std::vector<char> _encryptedData = std::vector<char>(cipher.begin(), cipher.end());

		return reinterpret_cast<std::vector<unsigned char>&>(_encryptedData);

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
std::vector<unsigned char> XOR(std::vector<unsigned char> plaintext) {
	if (debug) {
		debug_message("XOR - Sigflip", "[DEBUG]");
	}
	std::vector<unsigned char> result(plaintext.size());

	for (int i = 0; i < plaintext.size(); i++) {
		result[i] = (plaintext[i] ^ xor_key);
	}
	return result;
}


BOOL Sha1(BYTE* peblob, char* sha1Buf, DWORD dwBufferLen)
{
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;
	BYTE rgbHash[SHA1LEN];
	DWORD cbHash = 0;

	// Get handle to the crypto provider
	if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)){
		return FALSE;
	}

	if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash)) {
		CryptReleaseContext(hProv, 0);
		return FALSE;
	}

	if (!CryptHashData(hHash, peblob, dwBufferLen, 0)){	
		CryptReleaseContext(hProv, 0);
		CryptDestroyHash(hHash);
		return FALSE;
	}


	cbHash = SHA1LEN;
	if (CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0)) {
		for (DWORD i = 0; i < cbHash; i++) {
			sprintf( sha1Buf + (i * 2), "%02x", rgbHash[i]);
		}
	}
	else {
		return FALSE;
	}

	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);

	return TRUE;
}


BOOL IsWow64(HANDLE pHandle)
{
	BOOL isWow64 = FALSE;

	typedef BOOL(WINAPI *PFNIsWow64Process) (HANDLE, PBOOL);
	PFNIsWow64Process _FNIsWow64Process;
	_FNIsWow64Process = (PFNIsWow64Process)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

	if (NULL != _FNIsWow64Process){
		if (!_FNIsWow64Process(pHandle, &isWow64)) {}
	}
	return isWow64;
}

void toMultiByte(DWORD strLen, CHAR* _Str, LPWSTR _wStr) {
	DWORD wlen = strLen * 2;
	_wStr = (LPWSTR)malloc(wlen * sizeof(wchar_t));
	mbstowcs(_wStr, _Str, strlen(_Str) + 1);
}


DWORD VerifyPESignature(PCWSTR FileName, HANDLE FileHandle)
{
	DWORD Error = ERROR_SUCCESS;
	bool WintrustCalled = false;
	GUID GenericActionId = WINTRUST_ACTION_GENERIC_VERIFY_V2;
	WINTRUST_DATA WintrustData = {};
	WINTRUST_FILE_INFO FileInfo = {};
	WINTRUST_SIGNATURE_SETTINGS SignatureSettings = {};

	// Setup data structures for calling WinVerifyTrust 
	WintrustData.cbStruct = sizeof(WINTRUST_DATA);
	WintrustData.dwStateAction = WTD_STATEACTION_VERIFY;
	WintrustData.dwUIChoice = WTD_UI_NONE;
	WintrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
	WintrustData.dwUnionChoice = WTD_CHOICE_FILE;

	FileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO_);
	FileInfo.hFile = FileHandle;
	FileInfo.pcwszFilePath = FileName;
	WintrustData.pFile = &FileInfo;

	SignatureSettings.cbStruct = sizeof(WINTRUST_SIGNATURE_SETTINGS);
	SignatureSettings.dwFlags = WSS_GET_SECONDARY_SIG_COUNT | WSS_VERIFY_SPECIFIC;
	SignatureSettings.dwIndex = 0;
	WintrustData.pSignatureSettings = &SignatureSettings;

	Error = WinVerifyTrust(NULL, &GenericActionId, &WintrustData);
	WintrustCalled = true;
	if (Error != ERROR_SUCCESS)
	{
		goto Cleanup;
	}

	// Now attempt to verify all secondary signatures that were found 
	for (DWORD x = 1; x <= WintrustData.pSignatureSettings->cSecondarySigs; x++)
	{

		// Need to clear the previous state data from the last call to WinVerifyTrust 
		WintrustData.dwStateAction = WTD_STATEACTION_CLOSE;
		Error = WinVerifyTrust(NULL, &GenericActionId, &WintrustData);
		if (Error != ERROR_SUCCESS)
		{
			//No need to call WinVerifyTrust again 
			WintrustCalled = false;
			goto Cleanup;
		}

		WintrustData.hWVTStateData = NULL;

		// Caller must reset dwStateAction as it may have been changed during the last call 
		WintrustData.dwStateAction = WTD_STATEACTION_VERIFY;
		WintrustData.pSignatureSettings->dwIndex = x;
		Error = WinVerifyTrust(NULL, &GenericActionId, &WintrustData);
		if (Error != ERROR_SUCCESS)
		{
			goto Cleanup;
		}
	}

Cleanup:

	// Caller must call WinVerifyTrust with WTD_STATEACTION_CLOSE to free memory 
	// allocate by WinVerifyTrust 
	if (WintrustCalled != false)
	{
		WintrustData.dwStateAction = WTD_STATEACTION_CLOSE;
		WinVerifyTrust(NULL, &GenericActionId, &WintrustData);
	}


	return Error;
}




