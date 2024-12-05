#include "module.hpp"
#include "runner.hpp"

char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
    return std::find(begin, end, option) != end;
}

bool runDylib(std::string filename){

    // Check & remove environment variables (if applicable)
    if (getenv("fn") != nullptr) {
        if (unsetenv("fn") != 0) {
            std::cerr << "Failed to unset environment variable fn" << std::endl;
            return 1;
        }
    }  
    if (getenv("ffn") != nullptr) {
        if (unsetenv("ffn") != 0) {
            std::cerr << "Failed to unset environment variable ffn" << std::endl;
            return 1;
        }
    }

    if (!fs::exists(filename)) {
        std::cerr << "Dylib " << filename.c_str() << " does not exist!" << std::endl;
        return false;
    }

    void* handle = dlopen(filename.c_str(), RTLD_LAZY);
    if (handle == NULL) {
        std::cerr << "Could not obtain a handle to the dylib: " << dlerror() << std::endl;
        return false;
    }

    func_ptr* Initialize = (func_ptr*)dlsym(handle, "Initialize");
    if (Initialize == NULL) {
        std::cerr << "Could not find Initialize function: " << dlerror() << std::endl;
        return false;
    }

    char* result = Initialize();
    if (dlclose(handle) != 0) {
        std::cerr << "Could not close handle: " << dlerror() << std::endl;
        return false;
    }
    
    // Verify environment variable is set (if applicable)
    char* envFN = std::getenv("fn");
    if (envFN != nullptr) {
        std::cout << filename << " set the environment variable `fn` to: " << envFN << std::endl;
    } 
    char* envFFN = std::getenv("ffn");
    if (envFFN != nullptr) {
        std::cout << filename << " set the environment variable ffn is set to: " << envFFN << std::endl;
    } 

    std::cout << "Result from Module: " << filename << "\n" << result << std::endl;
    
    return true;
}

int main(int argc, char* argv[]){
    std::string path = "";

    if(cmdOptionExists(argv, argv+argc, "-p"))
    {
        path = getCmdOption(argv, argv + argc, "-p");
    }
    else{
        path = "/tmp/ABCDEF.tmp";
    }
    bool successful = false;
    if (std::filesystem::exists(path)){
       successful = runDylib(path);
        if (!successful){
            std::cerr << "**ERROR** Dylib File did not execute: " << path << std::endl;
        }
    }  
    return 0;
}