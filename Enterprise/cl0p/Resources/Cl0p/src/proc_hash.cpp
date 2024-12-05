#include <iostream>
#include <string>
#include <vector>


std::vector<std::string> PROCESS_NAMES = {
    "MSFTESQL.EXE",
    "SQLAGENT.EXE",
    "SQLBROSWER.EXE",
    "SQLWRITER.EXE",
    "OCSSD.EXE",
    "DBSNMP.EXE",
    "SYNCTIME.EXE",
    "AGNTSVC.EXEISQLPLUSSVC.EXE",
    "XFSSVCCON.EXE",
    "SQLSERVR.EXE",
    "MYDESKTOPSERVICE.EXE",
    "OCAUTOUPDS.EXE",
    "AGNTSVC.EXEAGNTSVC.EXE",
    "AGNTSVC.EXEENCSVC.EXE",
    "FIREFOXCONFIG.EXE",
    "TBIRDCONFIG.EXE",
    "MYDESKTOPQOS.EXE",
    "OCOMM.EXE",
    "MYSQLD.EXE",
    "MYSQLD-NT.EXE",
    "MYSQLD-OPT.EXE",
    "DBENG50.EXE",
    "SQBCORESERVICE.EXE",
    "SCALC.EXE",
    "INFOPATH.EXE",
    "SBASE.EXE",
    "ONENOTE.EXE",
    "OUTLOOK.EXE",
    "SIMPRESS.EXE",
    "STEAM.EXE",
    "THEBAT.EXE",
    "THEBAT64.EXE",
    "THUNDERBIRD.EXE",
    "SDRAW.EXE",
    "SWRITER.EXE",
    "WORDPAD.EXE",
    "SOFFICE.BIN"
};
std::vector<std::string> DIR_NAMES = {
    "WINDOWS",
    "BOOT",
    "PROGRAM FILES",
    "PROGRAM FILES (X86)",
    "ALL USERS",
    "LOCAL SETTINGS",
    "PROGRAMDATA",
    "TOR BROWSER",
    "APPDATA",
    "CHROME",
    "SYSTEM VOLUME INFORMATION",
    "PERFLOGS",
    "MOZILLA",
    "OPERA",
    "GOOGLE",
    "WINDOWS NT",
    "MICROSOFT",
    "INTEL",
    "APPLICATION DATA",
    "INTERNET EXPLORER",
    "COMMON FILES",
    "$RECYCLE.BIN",
    "ALL USERS",
    "$WINDOWS.~BT",
    "$WINDOWS.~WS",
    "WINDOWS JOURNAL",
    "MSBUILD",
};
std::vector<std::string> FILE_NAMES = {
    "README!_CI0P!.TXT",
    "NTDLR",
    "BOOT.INI",
    "NTUSER.INI",
    "AUTOEXEC.BAT",
    "NTDETECT.COM",
    "DESKTOP.INI",
    "AUTORUN.INF",
    "NTUSER.DAT",
    "ICONCACHE.DB",
    "BOOTSECT.BAK",
    "NTUSER.DATA.LOG",
    "THUMBS.DB"
};
std::vector<std::string> EXT_NAMES = {
    ".C_I0P",
    ".DLL",
    ".EXE",
    ".SYS",
    ".OCX",
    ".LNK",
};

void CalculateProcessHashes(std::vector<std::string> nameList) {
    for (const std::string& str : nameList) {
        const std::hash<std::string> hasher;
        const auto hashResult = hasher(str);
        std::string message = std::format("    size_t{{{}}}, // {}", hashResult, str);
        std::cout << message << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        CalculateProcessHashes(PROCESS_NAMES);
        std::cout << "----------" << std::endl;
        CalculateProcessHashes(DIR_NAMES);
        std::cout << "----------" << std::endl;
        CalculateProcessHashes(FILE_NAMES);
        std::cout << "----------" << std::endl;
        CalculateProcessHashes(EXT_NAMES);
        std::cout << "----------" << std::endl;
    }
    else {
        std::vector<std::string> arguments(argv + 1, argv + argc);
        CalculateProcessHashes(arguments);
    }
    return 0;
}