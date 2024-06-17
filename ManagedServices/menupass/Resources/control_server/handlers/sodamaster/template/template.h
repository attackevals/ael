#pragma once

#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <random>
#include "resource.h"
#include "testing/XorString.h"

#define DEFAULT_PROCESS L"C:\\Windows\\System32\\cmd.exe /c "
#define DEFAULT_PIPE_PREFIX L"zz"
#define PIPE_IN_BUFFER 64*1024
#define PIPE_OUT_BUFFER 64*1024
#define TIMEOUT_SECONDS 60
#define WAIT_PERIOD 100
