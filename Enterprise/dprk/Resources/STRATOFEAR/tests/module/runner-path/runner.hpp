#include <dlfcn.h>
#include <string>
#include <iostream>
#include <cstdio>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;


typedef char* func_ptr();