#ifndef NAMEDPIPE_H_
#define NAMEDPIPE_H_

#include <windows.h>
#include <fileapi.h>
#include <iostream>
#include <stdio.h> 
#include <filesystem>
#include <vector>
#include "XorString.h"

#define PIPE_IN_BUFFER 64*1024
#define PIPE_OUT_BUFFER 64*1024
#define DEFAULT_PIPE_PREFIX "zz"

std::vector<std::string> FindPipeWithPrefix();
std::string IterNamedPipes(std::vector<std::string> namedPipes);

#endif