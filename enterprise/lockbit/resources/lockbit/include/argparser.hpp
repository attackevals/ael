/*
 * Provides a basic argument parser. Currently supports switches and string options.
 */

#pragma once

#include <format>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

// Reference: https://stackoverflow.com/a/868894
class ArgParser {
    public:
        ArgParser(int argc, char** argv) {
            // Start at index 1 to skip the program name
            for (int i = 1; i < argc; i++) {
                this->tokens.push_back(std::string(argv[i]));
            }
            this->accepts_standalone_values = false;
        }

        std::vector<std::string> GetArgTokens() {
            return std::vector<std::string>(this->tokens);
        }

        void AllowStandaloneValues() {
            this->accepts_standalone_values = true;
        }

        void SetFlag(const char* option) {
            std::string flag(option);
            if (!StringStartsWith(flag, "-")) {
                throw std::invalid_argument("Option must start with -");
            }
            if (this->allowed_flags.contains(flag) || this->allowed_string_args.contains(flag)) {
                throw std::invalid_argument(std::format("Cannot set option {} more than once", flag));
            }
            this->allowed_flags.insert(flag);
        }

        void SetStringArg(const char* option) {
            std::string flag(option);
            if (!StringStartsWith(flag, "-")) {
                throw std::invalid_argument("Option must start with -");
            }
            if (this->allowed_flags.contains(flag) || this->allowed_string_args.contains(flag)) {
                throw std::invalid_argument(std::format("Cannot set option {} more than once", flag));
            }
            this->allowed_string_args.insert(flag);
        }

        void Parse() {
            bool require_value = false;
            std::string option_requiring_value = "";
            for (std::string token: tokens) {
                if (StringStartsWith(token, "-")) {
                    // Process flag/option
                    if (require_value) {
                        throw std::invalid_argument(std::format("Value not provided for option {}", option_requiring_value));
                    }
                    if (this->processed_options.contains(token)) {
                        throw std::invalid_argument(std::format("Cannot provide option {} more than once", token));
                    }
                    if (this->allowed_flags.contains(token)) {
                        // Handle flag
                        require_value = false;
                        this->enabled_flags.insert(std::string(token));
                        this->processed_options.insert(std::string(token));
                    } else if (this->allowed_string_args.contains(token)) {
                        // Handle string option
                        require_value = true;
                        option_requiring_value = std::string(token);
                        this->processed_options.insert(std::string(token));
                    } else {
                        throw std::invalid_argument(std::format("Unrecognized option {}", token));
                    }
                } else {
                    if (require_value) {
                        // String arg value - next token cannot be a flag
                        require_value = false;
                        this->args_and_values[option_requiring_value] = std::string(token);
                    } else if (this->accepts_standalone_values) {
                        this->standalone_values.push_back(std::string(token));
                    } else {
                        throw std::invalid_argument(std::format("Standalone values not allowed. Was given: {}", token));
                    }
                }
            }
            
            // Check if we finished iterating through tokens and are missing an arg value
            if (require_value) {
                throw std::invalid_argument(std::format("Value not provided for option {}", option_requiring_value));
            }
        }

        bool GetFlag(const char* flag) {
            std::string option(flag);
            if (!StringStartsWith(option, "-")) {
                option = "-" + option;
            }
            return this->enabled_flags.contains(option);
        }

        std::string GetStringArg(const char* strOption) {
            std::string option(strOption);
            if (!StringStartsWith(option, "-")) {
                option = "-" + option;
            }
            return this->args_and_values[option];
        }

        std::vector<std::string> GetStandaloneValues() {
            return std::vector<std::string>(this->standalone_values);
        }

    private:
        bool accepts_standalone_values;
        std::vector<std::string> tokens;
        std::map<std::string, std::string> args_and_values;
        std::set<std::string> enabled_flags;
        std::vector<std::string> standalone_values;
        std::set<std::string> processed_options;
        std::set<std::string> allowed_flags;
        std::set<std::string> allowed_string_args;

        bool StringStartsWith(std::string input, std::string substring) {
            return input.rfind(substring, 0) == 0;
        }
};
