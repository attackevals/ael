#include<iostream>
#include<fstream>

using namespace std;
std::string CONFIG_FILENAME = "config";
std::string CERT_FILEPATH = CERT_PATH_ENV;
std::string HEADER_FILEPATH = HEADER_PATH_ENV;

//Article for parsing command line flags: https://stackoverflow.com/questions/865668/parsing-command-line-arguments-in-c/868894#868894
char* getCmdOption(char ** begin, char ** end, const string & option)
{
    char ** itr = find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const string& option)
{
    return find(begin, end, option) != end;
}


int main(int argc, char* argv[]) {
    //Default values 
    char uuid[3]={0x67, 0x69, 0x64};
    char version[2]={0x6c, 0x65};
    char * domain;

    if(cmdOptionExists(argv, argv+argc, "-d"))
    {

        domain = getCmdOption(argv, argv + argc, "-d");

        cout << "Creating the STRATOFEAR network configuration file to use domain/port " << domain << endl;
    }
    else{
        domain = new char[strlen("pssl://127.0.0.1:8080") + 1]; 
        strcpy(domain, "pssl://127.0.0.1:8080");
        cout << "No command arguments provided. Creating the STRATOFEAR network configuration file using default localhost, " << domain << "." << endl;
    }
    
    ofstream configFile(CONFIG_FILENAME, ios::out | ios::binary);
    if(!configFile) {
        cout << "Cannot open file!" << endl;
    }
    
    //insert values into the file at offsets
    configFile.seekp(410, ios::beg);
    configFile.write((const char*)uuid, sizeof(uuid));
    configFile.seekp(416, ios::beg);
    configFile.write((const char*)version, sizeof(version));
    configFile.seekp(526, ios::beg);
    configFile.write((const char*)domain, strlen(domain));
    configFile.close();

    if(!configFile.good()) {
        cout << "Error occurred at writing time!" << endl;
    }

    //create header file with HTTPS cert
    std::string command = "xxd -i " + CERT_FILEPATH + " > " + HEADER_FILEPATH;
    if (std::system(command.c_str()) != 0) {
        cout << "Error occurred creating header file!" << endl;
        return 1;
    }

    //replace the variable types & names
    std::string temp = CERT_FILEPATH;
    std::replace(temp.begin(), temp.end(), '/', '_');
    command = "sed -i '' -e 's/unsigned char " + temp + "/extern char pem/' " + HEADER_FILEPATH;
    std::system(command.c_str());
    command = "sed -i '' -e 's/unsigned int " + temp + "/extern int pem/' " + HEADER_FILEPATH;
    std::system(command.c_str());

    return 0;
}