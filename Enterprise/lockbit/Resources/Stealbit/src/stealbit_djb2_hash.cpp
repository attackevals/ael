#include <iostream>
#include <string>
#include <vector>

inline unsigned long Djb2HashStringA(const std::string& str) {
    std::string SEED = DJB2_HASH_SEED;
    unsigned long hash = stoi(SEED);
    int c;

    for (int i = 0; i < str.length(); ++i) {
        c = (int)str[i];
        hash = ((hash << 9) + hash) + c;
    }

    return hash;
}


std::vector<std::string> list = {
        "appdata",
        "AppData",
        ".386",
        ".adv",
        ".ani",
        ".apk",
        ".app",
        ".bat",
        ".bin",
        ".cmd",
        ".com",
        ".cpl",
        ".cur",
        ".css",
        ".diagcab",
        ".diagcfg",
        ".diagpkg",
        ".dll",
        ".dmg",
        ".dmp",
        ".drv",
        ".exe",
        ".fnt",
        ".fon",
        ".gadget",
        ".hlp",
        ".hta",
        ".icns",
        ".ico",
        ".ics",
        ".idx",
        ".ini",
        ".ipa",
        ".iso",
        ".key",
        ".lnk",
        ".lock",
        ".lockbit",
        ".mod",
        ".mp3",
        ".mp4",
        ".mpa",
        ".msc",
        ".msi",
        ".msp",
        ".msstyles",
        ".msu",
        ".nls",
        ".ocx",
        ".otf",
        ".part",
        ".pif",
        ".prf",
        ".ps1",
        ".rdp",
        ".reg",
        ".rom",
        ".rtp",
        ".sfcache",
        ".shs",
        ".spl",
        ".sys",
        ".theme",
        ".tmp",
        ".ttf",
        ".wad",
        ".wav",
        ".wma",
        ".woff",
        ".wpx",
        ".xex"
};

void CreateHash(const std::vector<std::string>& list) {
    for (const std::string& str : list) {
        std::cout << std::format("{}, // {}", Djb2HashStringA(str), str) << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        CreateHash(list);
    }
    else {
        std::vector<std::string> arguments(argv + 1, argv + argc);
        CreateHash(arguments);
    }
    return 0;
}