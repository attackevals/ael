#include "crypto/encryption.hpp"
#include "crypto/xor_obfuscation.hpp"
#include <format>
#include <stdexcept>
#include "util/winapihelper.hpp"

namespace encryption {

/*
 * GetRandBytes:
 *      About:
 *          Wrapper for BCryptGenRandom API call that fills buffer with n random bytes.
 *          Assumes buffer can store n bytes.
 *      Result:
 *          buffer filled with n random bytes, or throws exception on failure.
 */
void GetRandBytes(unsigned char* buffer, size_t n) {
    NTSTATUS result = BCryptGenRandom(BCRYPT_RNG_ALG_HANDLE, buffer, n, 0);
    if (result == STATUS_INVALID_HANDLE) {
        throw std::invalid_argument(XOR_LIT("BCryptGenRandom: the handle in the hAlgorithm parameter is not valid."));
    } else if (result == STATUS_INVALID_PARAMETER) {
        throw std::invalid_argument(XOR_LIT("BCryptGenRandom: One or more parameters are not valid."));
    } else if (result != STATUS_SUCCESS) {
        throw std::runtime_error(std::format("{} {}", XOR_LIT("BCryptGenRandom failed with NTSTATUS"), result));
    }
}

/*
 * AesCtrCrypt:
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
void AesCtrCrypt(unsigned char* key, unsigned char* iv, unsigned char* buffer, size_t buf_len) {
    /* Removed for public release */
    return;
}

/*
 * RsaOaepSha1Encrypt:
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
 *      Other References:
 *          https://www.cryptopp.com/wiki/RSA_Cryptography#Sample_Programs
 */
std::vector<unsigned char> RsaOaepSha1Encrypt(const std::vector<unsigned char>& v_plaintext, const std::vector<unsigned char>& key) {
    /* Removed for public release */
    return v_plaintext;
}

/*
 * RsaOaepSha1Decrypt:
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
 *      Other References:
 *          https://www.cryptopp.com/wiki/RSA_Cryptography#Sample_Programs
 */
std::vector<unsigned char> RsaOaepSha1Decrypt(const std::vector<unsigned char>& v_ciphertext, const std::vector<unsigned char>& key) {
    /* Removed for public release */
    return v_ciphertext;
}

/*
 * RsaOaepSha1DecryptWithBase64Key:
 *      About:
 *          Decrypts the RSA ciphertext using the provided base64-encoded RSA private key. 
 *          Uses OAEP padding and SHA1 hashing. Leverages Crypto++ library.
 *      Result:
 *          RSA-decrypted plaintext message
 *      Returns:
 *          Unsigned char vector containing the resulting plaintext
 *      MITRE ATT&CK Techniques:
 *          T1140: Deobfuscate/Decode Files or Information
 *      Other References:
 *          https://www.cryptopp.com/wiki/Keys_and_Formats#Loading_Keys
 *          https://www.cryptopp.com/wiki/RSA_Cryptography#Sample_Programs
 */
std::vector<unsigned char> RsaOaepSha1DecryptWithBase64Key(const std::vector<unsigned char>& v_ciphertext, const std::string& base64_key) {
    /* Removed for public release */
    return v_ciphertext;
}


/*
 * EncryptRsaPrivateSessionKey:
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
std::vector<unsigned char> EncryptRsaPrivateSessionKey(const std::vector<unsigned char>& enc_key, const std::vector<unsigned char>& private_key) {
    /* Removed for public release */
    return private_key;
}

/*
 * DecryptRsaPrivateSessionKey:
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
std::vector<unsigned char> DecryptRsaPrivateSessionKey(const std::vector<unsigned char>& key, const std::vector<unsigned char>& ciphertext) {
    /* Removed for public release */
    return ciphertext;
}

} // namespace
