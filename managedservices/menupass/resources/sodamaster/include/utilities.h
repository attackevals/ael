#pragma once
#include <Windows.h>
#include <chrono>
#include <ctime> 
#include <iostream>
#include <fstream>
#include <filesystem>
#include "XorString.h"

int RetCharVal(int val1, int val2);

std::string currentDateTime();

std::string retHex(std::string data);

std::string decimalToHex(int decimal_value);

std::string hexToString(const std::string& hex_data);

int hexToDecimal(std::string hex_value);

int strToInt(std::string strValue);

int hexStrToInt(std::string hex_data);

std::string intToHexBytes(int length);

std::vector<char> HexToBytes(std::string hex);

int writeToFile(std::string text, std::string log_type="debug");