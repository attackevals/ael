#pragma once
#ifndef __MAIN_H__
#define __MAIN_H__
#pragma once

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <numeric>



// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 1024

#include <Windows.h>
#include <Lmcons.h>
#include <chrono>
#include <thread>
#include <ctime> 
#include <string>
#include <iostream>
#include <vector>
#include <numeric>
#include "Core.h"
#include "Utilities.h"
#include "AVM.h"
#include "Comslib.h"
#include "XorString.h"
#include "EncUtils.h"
#include "NamedPipe.h"


#endif