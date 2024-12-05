/*Good reference for integrating with cmake https://discourse.cmake.org/t/cmake-cant-find-library-header-files/1334*/

#include "module.hpp"
std::string USERNAME = MODULE_USERNAME;

char* setEnvironmentVariables(const std::vector<std::pair<std::string, std::string>>& envVariableMap) {

    std::string combinedVariableNames;


    // Loop through the map and set environment variables
    for (const auto& pair : envVariableMap) {
        const char* envVarName = pair.first.c_str();
        const char* envVarValue = pair.second.c_str();
        if (setenv(envVarName, envVarValue, 1) != 0) {
            return nullptr;
        }
        combinedVariableNames += pair.first + " ";
    }

    // Remove the last space
    if (!combinedVariableNames.empty()) {
        combinedVariableNames.pop_back();
    }

    char* result = new char[combinedVariableNames.length() + 1];
    std::strcpy(result, combinedVariableNames.c_str());
    return result;
}

char* Initialize(){

    // Mapping of environment variables for keychain file paths
    std::vector<std::pair<std::string, std::string>> envVariablesMap = {
        {"JC_BUNDLE_ID", "/Users/" + USERNAME + "/Library/Keychains/login.keychain-db"},
        {"JC_WORKFLOW_MSG", "/Library/Keychains/System.keychain"}
    };

    // Set environment variables & return a point with the variables used
    char* envVarCharPtr = setEnvironmentVariables(envVariablesMap);
    if (envVarCharPtr) {
        return envVarCharPtr;
    }

    const char* failureMessage = "Fail";
    char* message = new char[strlen(failureMessage) + 1];
    std::strcpy(message, failureMessage);
    return message;
}