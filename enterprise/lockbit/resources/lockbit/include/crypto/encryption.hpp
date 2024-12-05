#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <bcrypt.h>
#include <vector>
#include <string>

namespace encryption {

/*
 * GetRandBytes (defined in src/common/encryption.cpp):
 *      About:
 *          Wrapper for BCryptGenRandom API call that fills buffer with n random bytes.
 *          Assumes buffer can store n bytes.
 *      Result:
 *          buffer filled with n random bytes, or throws exception on failure.
 */
void GetRandBytes(unsigned char* buffer, size_t n);

/*
 * AesCtrCrypt (defined in src/common/encryption.cpp):
 *      About:
 *          Performs AES-CTR encryption/decryption using the provided key, IV, and input buffer. Encryption and
 *          decryption is the same routine and is performed in-place in the provided buffer.
 *      Result:
 *          AES-CTR encrypted/decrypted input buffer.
 *      MITRE ATT&CK Techniques:
 *          T1027: Obfuscated Files or Information
 *      CTI:
 *          https://redpiranha.net/news/look-lockbit-3-ransomware
 *          https://www.trendmicro.com/vinfo/us/security/news/ransomware-spotlight/ransomware-spotlight-lockbit
 */
void AesCtrCrypt(unsigned char* key, unsigned char* iv, unsigned char* buffer, size_t buf_len);

/*
 * RsaOaepSha1Encrypt (defined in src/common/encryption.cpp):
 *      About:
 *          RSA-encrypts the provided plaintext using OAEP padding and SHA1 hashing. Will chunk long plaintexts
 *          and will concatenate the resulting ciphertexts.
 *          Leverages Crypto++ library.
 *      Result:
 *          Ciphertext from RSA encryption
 *      Returns:
 *          Unsigned char vector containing the RSA-encrypted plaintext.
 *      MITRE ATT&CK Techniques:
 *          T1027: Obfuscated Files or Information
 *      CTI:
 *          https://redpiranha.net/news/look-lockbit-3-ransomware
 */
std::vector<unsigned char> RsaOaepSha1Encrypt(const std::vector<unsigned char>& v_plaintext, const std::vector<unsigned char>& key);

/*
 * RsaOaepSha1Decrypt (defined in src/common/encryption.cpp):
 *      About:
 *          RSA-decrypts the provided ciphertext using OAEP padding and SHA1 hashing. Will chunk long ciphertexts
 *          and will concatenate the resulting plaintext messages.
 *          Leverages Crypto++ library.
 *      Result:
 *          Original plaintext from RSA ciphertext
 *      Returns:
 *          Unsigned char vector containing the RSA-decrypted plaintext.
 *      MITRE ATT&CK Techniques:
 *          T1140: Deobfuscate/Decode Files or Information
 *      CTI:
 *          https://redpiranha.net/news/look-lockbit-3-ransomware
 */
std::vector<unsigned char> RsaOaepSha1Decrypt(const std::vector<unsigned char>& v_ciphertext, const std::vector<unsigned char>& key);

/*
 * RsaOaepSha1DecryptWithBase64Key (defined in src/common/encryption.cpp):
 *      About:
 *          Decrypts the RSA ciphertext using the provided base64-encoded RSA private key. 
 *          Uses OAEP padding and SHA1 hashing. Leverages Crypto++ library.
 *      Result:
 *          RSA-decrypted plaintext message
 *      Returns:
 *          Unsigned char vector containing the resulting plaintext
 *      MITRE ATT&CK Techniques:
 *          T1140: Deobfuscate/Decode Files or Information
 */
std::vector<unsigned char> RsaOaepSha1DecryptWithBase64Key(const std::vector<unsigned char>& v_ciphertext, const std::string& base64_key);

/*
 * EncryptRsaPrivateSessionKey (defined in src/common/encryption.cpp):
 *      About:
 *          RSA-encrypts the provided RSA private session key using OAEP padding and SHA1 hashing. 
 *          Leverages Crypto++ library.
 *      Result:
 *          Encrypted RSA private session key
 *      Returns:
 *          Unsigned char vector containing the RSA-encrypted private session key.
 *      MITRE ATT&CK Techniques:
 *          T1027: Obfuscated Files or Information
 *      CTI:
 *          https://redpiranha.net/news/look-lockbit-3-ransomware
 *          https://blog.lexfo.fr/lockbit-malware.html
 *      Other References:
 *          https://www.cryptopp.com/wiki/Keys_and_Formats#High_Level_Objects
 */
std::vector<unsigned char> EncryptRsaPrivateSessionKey(const std::vector<unsigned char>& enc_key, const std::vector<unsigned char>& private_key);

/*
 * DecryptRsaPrivateSessionKey (defined in src/common/encryption.cpp):
 *      About:
 *          RSA-decrypts the encrypted RSA private session key using OAEP padding and SHA1 hashing. 
 *          Leverages Crypto++ library.
 *      Result:
 *          Decrypted RSA private session key
 *      Returns:
 *          Decrypted RSA private session key
 *      MITRE ATT&CK Techniques:
 *          T1140: Deobfuscate/Decode Files or Information
 *      CTI:
 *          https://redpiranha.net/news/look-lockbit-3-ransomware
 *          https://blog.lexfo.fr/lockbit-malware.html
 *      Other References:
 *          https://www.cryptopp.com/wiki/Keys_and_Formats#High_Level_Objects
 */
std::vector<unsigned char> DecryptRsaPrivateSessionKey(const std::vector<unsigned char>& key, const std::vector<unsigned char>& ciphertext);

} // namespace