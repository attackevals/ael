#pragma once
#ifndef __HASH__HPP
#define __HASH__HPP
#include "settings.hpp"

namespace Hashing {

    /*
    * Djb2HashStringA:
    *      About:
    *          Takes a string arg and hashes it using the predefined hash seed
    *      Result:
    *          djb2 string hash
    *      Returns:
    *          unsigned long
    *      MITRE ATT&CK Techniques:
    *          n/a
    *      CTI:
    *          n/a
    *      Other References:
    *          n/a
    */
    inline unsigned long Djb2HashStringA(const std::string& str) {
        std::string SEED = settings::DJB2_SEED;
        unsigned long hash = stoi(SEED);
        int c;

        for (int i = 0; i < str.length(); ++i) {
            c = (int)str[i];
            hash = ((hash << 9) + hash) + c;
        }

        return hash;
    }
}
#endif // !__HASH__HPP