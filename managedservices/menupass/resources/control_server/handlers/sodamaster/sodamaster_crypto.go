package sodamaster

import (
    "crypto/rand"
    "crypto/rc4"
    "crypto/rsa"
    "crypto/sha1"
    "crypto/x509"
    "encoding/base64"
    "errors"
    "unicode/utf8"
)

// importRsaPrivKey imports the base64 encoded server's RSA private key used to decrypt the implant comms
//
// returns the imported RSA private key
func ImportRsaPrivKey() (*rsa.PrivateKey, error) {
    keyData, err := base64.StdEncoding.DecodeString(serverPrivateKey)
    if err != nil {
        return nil, err
    }
    privateKey, err := x509.ParsePKCS1PrivateKey(keyData)
    if err != nil {
        return nil, err
    }
    return privateKey, nil
}

// rsaDecrypt decrypts the RSA encrypted input (sha1 hash function, OAEP padding, and 2048-bit key). Source: https://golangdocs.com/rsa-encryption-decryption-in-golang
//
//	cipherText: []byte that represents the encrypted data
//	privKey: the servers RSA private key that is used to decrypt the data
//
// returns the plain text and any errors
func rsaDecrypt(encryptedData string, privKey *rsa.PrivateKey) ([]byte, error) {
    rng := rand.Reader
    cipherText, _ := base64.StdEncoding.DecodeString(encryptedData)
    plainText, err := rsa.DecryptOAEP(sha1.New(), rng, privKey, cipherText, nil)
    if err != nil {
        return nil, err
    }
    return plainText, nil
}

// rc4Encrypt encrypts the provided data with the provided RC4 key
//
//	plaintext: byte array representing the plain text data to be encrypted, rc4Key: RC4 key used to encrypt the data
//
// returns the encrypted data and any errors
func rc4Encrypt(plaintext []byte, rc4Key []byte) (string, error) {
    //Initialize the cipher
    cipher, err := rc4.NewCipher(rc4Key)
    if err != nil {
        return "", err
    }

    //Base64 enocde the data, encrypt it, and base64 encode it again
    b64Plaintext := base64.StdEncoding.EncodeToString(plaintext)
    ciphertext := make([]byte, len(b64Plaintext))
    cipher.XORKeyStream(ciphertext, []byte(b64Plaintext))
    b64Ciphertext := base64.StdEncoding.EncodeToString(ciphertext)

    return b64Ciphertext, nil
}

// rc4Decrypt decrypts the provided data with the provided RC4 key
//
//	ciphertext: byte array representing the cipher text data to be decrypted, rc4Key: RC4 key used to decrypt the data
//
// returns the plain text and any errors
func rc4Decrypt(encryptedData string, rc4Key []byte) (string, error) {
    //Initialize the cipher
    cipher, err := rc4.NewCipher(rc4Key)
    if err != nil {
        return "", err
    }

    //Base64 decode the data and then decrypt it
    ciphertext, _ := base64.StdEncoding.DecodeString(encryptedData)
    b64plaintext := make([]byte, len(ciphertext))
    cipher.XORKeyStream(b64plaintext, ciphertext)

    //Check if decrypted data is valid and base64 decode it if so
    if !utf8.Valid(b64plaintext) {
        return "", errors.New("Decryption unsuccessful")
    }
    plaintext, _ := base64.StdEncoding.DecodeString(string(b64plaintext))

    return string(plaintext), nil
}

// decrypt takes the encrypted data sent from the implant and attempts to decrypt it with all the available keys
//
//	encryptedData: a string representing the encrypted data sent from the implant, privKey: the static RSA private key used to decrypt the implant's first packet
//	rc4Keys: map containing all of the RC4 keys belonging to the registered implants used to decrypt their subsequent packets
//
// returns the decrypted plain text, a bool indicating if the data represents the implants first packet, and any errors if they occur
func decrypt(encryptedData string, privKey *rsa.PrivateKey, rc4Keys map[string][]byte) (string, bool, error) {
    firstPacket := false

    //Try to decrypt the data with the RSA key, if successful return early and mark the data as the first packet
    b64PlainText, err := rsaDecrypt(encryptedData, privKey)
    if err == nil {
        firstPacket = true
        plainText, _ := base64.StdEncoding.DecodeString(string(b64PlainText))
        return string(plainText), firstPacket, err
    }

    //RSA decryption did not work- now we try all the RC4 keys
    var plaintext string
    for _, rc4Key := range rc4Keys {
        //Try to decrypt and return early if successful
        plaintext, err = rc4Decrypt(encryptedData, rc4Key)
        if err == nil {
            return plaintext, firstPacket, nil
        }
    }

    return "", firstPacket, errors.New("Decryption unsuccessful: Invalid data or no valid keys.")
}
