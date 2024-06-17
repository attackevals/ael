#ifndef NAMEDPIPE_H_
#define NAMEDPIPE_H_

#include <windows.h>
#include <errhandlingapi.h>
#include <fileapi.h>
#include <iostream>
#include <stdio.h> 
#include <tchar.h>
#include <strsafe.h>
#include <filesystem>
#include "XorString.h"
#include "Utilities.h"

 // https://learn.microsoft.com/en-us/windows/win32/api/namedpipeapi/nf-namedpipeapi-transactnamedpipe
 // "The maximum guaranteed size of a named pipe transaction is 64 kilobytes."
#define PIPE_IN_BUFFER 64*1024
#define PIPE_OUT_BUFFER 64*1024
#define DEFAULT_PIPE_PREFIX "zz"


std::vector<std::string> FindPipeWithPrefix();
std::string IterNamedPipes(std::vector<std::string> namedPipes);

#endif