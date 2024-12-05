#include "config.hpp"
#include "crypto/xor_obfuscation.hpp"
#include "resource.hpp"
#include "util/resource_util.hpp"
#include <stdexcept>

/*
 * GetConfig:
 *      About:
 *          Fetches the configuration data stored as a resource and parses it into a Config object.
 *      Result:
 *          Returns Config object or throws exception on error.
 */
Config GetConfig() {
    std::string config_str = "{}";
    try {
        std::vector<char> config_buf = resource_util::ReadResource(CONFIG_RESOURCE_ID);
        config_str = std::string(config_buf.begin(), config_buf.end());
    } catch (const std::exception& e) {
        throw std::runtime_error(std::format("{}: {}", XOR_LIT("Failed to get config resource data"), e.what()));
    } catch (...) {
        throw std::runtime_error(XOR_LIT("Unknown exception when getting config resource data."));
    }
    return Config(config_str.c_str());
}