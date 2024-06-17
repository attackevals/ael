#ifndef __AVM_H__
#define __AVM_H__
#pragma once
#include <Windows.h>
#include <chrono>
#include <ctime> 
#include <iostream>
#include <stdlib.h>
#include <intrin.h>
#include "Core.h"
#include "Utilities.h"


#define HV_MSR_RANGE_START 0x40000000

// Holds the output of '__rdtsc()' CPU instrucion
#define LODWORD(_qw)    ((DWORD)(_qw))
#define HIDWORD(_qw)    ((DWORD)(((_qw) >> 32) & 0xffffffff))

// CheckCoreCount()
#define CORE_CHECK_ERROR 101 

// CheckVMProcess()
#define PROC_CHECK_ERROR 102 

// CheckCPUFlag()
#define CPU_FLAG_ERROR	 104

// CheckExpectedTime()
#define CHECK_TIME_ERROR 105

// RegistryCheck()
#define REG_CHECK_ERROR	 106

// MSRCheck()
#define MSR_CHECK_ERROR	 107


BOOL CheckExpectedTime(std::chrono::duration<double> duration, std::chrono::duration<double> lastDuration, int sleep_time);

std::string CheckProcCount();

std::string CheckVMProcess();

std::string RegistryCheck(HKEY hkey, LPCSTR path);

VOID MSRCheck();


#endif