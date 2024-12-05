#include "core.hpp"


namespace Core {
    bool CompareHash(std::string value, std::vector<size_t> hashList) {
        // Convert to uppercase
        for (std::string::iterator::value_type& c : value) c = (unsigned char)toupper(c);

        // Compare hash to predefined list
        const std::hash<std::string> hasher;
        const size_t hashResult = hasher(value);
        const std::vector<size_t>::iterator found = std::find(hashList.begin(), hashList.end(), hashResult);

        return found != hashList.end();
    }
}