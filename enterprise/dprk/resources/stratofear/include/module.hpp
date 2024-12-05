#ifndef module_hpp
#define module_hpp

#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <filesystem>
#include <cstring>
#include <thread>
#include <pwd.h>
#include <CoreServices/CoreServices.h>
#include "XorLogger.hpp"

extern std::string USERNAME;

#ifdef __cplusplus
extern "C" {
#endif

char* Initialize();

#ifdef __cplusplus
} 
#endif

#endif