#include "argparser.hpp"


namespace arguments {

    std::string startingPoint;
    std::string sbFileName;
    bool selfDelete = false;
    bool allowAllExt = false;
    bool allowAllDir = false;
    long fileSize = 0;


    void InitArgs(int argc, char* argv[])
    {
        // Parse command lines
        ArgParser args(argc, argv);
        args.SetFlag(XOR_LIT("-delete")); // Toggle self-delete
        args.SetFlag(XOR_LIT("-d")); // Toggle self-delete
        args.SetFlag(XOR_LIT("-allowallext")); // Toggle to not skip any file extension
        args.SetStringArg(XOR_LIT("-file")); // To specify the maximum file size
        args.SetStringArg(XOR_LIT("-f")); // To specify the maximum file size
        args.SetFlag(XOR_LIT("-allowalldir")); // Toggle to not skil any directories

        // allow standalone so we can get our dir name
        args.AllowStandaloneValues();

        try {
            args.Parse();
        }
        catch (std::exception& e) {
            XorLogger::LogError(XOR_LIT("Failed to parse command-line args"));
        }
        if (argc <= 1) 
        {
            arguments::startingPoint = "C:\\";
        }
        else
        {
            arguments::startingPoint = argv[1];
        }
        arguments::sbFileName = argv[0];

        // toggle self delete
        if (args.GetFlag(XOR_LIT("-d")) || args.GetFlag(XOR_LIT("-delete"))) {
            XorLogger::LogInfo(XOR_LIT("Enabled self-delete"));
            arguments::selfDelete = true;
        }
        else {
            XorLogger::LogInfo(XOR_LIT("Disabled self-delete"));
        }

        // allow all extensions
        if (args.GetFlag(XOR_LIT("-allowallext"))) {
            XorLogger::LogInfo(XOR_LIT("Disabled extension blacklist"));
            arguments::allowAllExt = true;
        }
        else {
            XorLogger::LogInfo(XOR_LIT("Enabled extension blacklist"));
        }

        // allow all directories
        if (args.GetFlag(XOR_LIT("-allowalldir"))) {
            XorLogger::LogInfo(XOR_LIT("Disabled directory blacklist"));
            arguments::allowAllDir = true;
        }
        else {
            XorLogger::LogInfo(XOR_LIT("Enabled directory blacklist"));
        }

        if (args.GetStringArg(XOR_LIT("-f")).length() > 0) {
            arguments::fileSize = stol(args.GetStringArg(XOR_LIT("-f")));
            XorLogger::LogInfo(XOR_LIT("Setting file size to: ") + args.GetStringArg(XOR_LIT("-f")));
        }
        else if (args.GetStringArg(XOR_LIT("-file")).length() > 0) {
            arguments::fileSize = stol(args.GetStringArg(XOR_LIT("-file")));
            XorLogger::LogInfo(XOR_LIT("Setting file size to: ") + args.GetStringArg(XOR_LIT("-file")));
        }
        else {
            XorLogger::LogInfo(XOR_LIT("Using default file size of 2MB"));
        }
    }
};