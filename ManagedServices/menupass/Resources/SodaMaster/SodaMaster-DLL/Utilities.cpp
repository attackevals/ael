#include  "Utilities.h"

//  RetCharVal(int val1, int val2)
//      About:
//         takes two integer values and does some math operations with them. The final result is val1 + val2 - 5
//      Result:
//          Returns an integer value that matches a desired decimal character value
//      MITRE ATT&CK Techniques:
//          T1027 Obfuscated Files or Information
//      CTI:
//          http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf
int RetCharVal(int val1, int val2) {
	int val3 = val1 * 100;
	int val4 = val2 * 50;
	int val5 = 0;
	if (val3 > val1) {
		val3 = val3 / 25;
		val5 = val3 / 4;
		val5 = val5 - 5;
	}
	return val5 + (val4 / 50);
}
// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
std::string currentDateTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
	// for more information about date/time format
	strftime(buf, sizeof(buf), XorStr("%Y-%m-%d %X"), &tstruct);

	return buf;
}

// Returns the hex value for given data
std::string retHex(std::string data) {
	std::ostringstream os;

	for (int i = 0; i < data.length(); i++)
		os << std::hex << std::uppercase << (int)data[i];

	std::string hexdata = os.str();
	return hexdata;
}

// Converts a decimal value to hex
// eg. 255 -> FF
std::string decimalToHex(int decimal_value) {
	std::stringstream ss;
	ss << std::hex << decimal_value; // int decimal_value
	std::string res(ss.str());
	std::string rValue;
	if (res.length() == 1) {
		rValue = "0";
		rValue += res;
		return rValue;
	}
	return res;
	}

// Converts from a hex string value to an integer
int hexStrToInt(std::string hex_data) {
	std::string placeholder = "";
	for (auto i = 0; i < hex_data.length(); i+=2)
	{
		std::stringstream ss;
		int decimal_value;
		ss << hex_data.substr(i, 2); // std::string hex_value
		ss >> std::hex >> decimal_value; //int decimal_value

		placeholder += std::to_string(decimal_value);
	}
	int integerValue = std::stoi(placeholder);
	return integerValue;
}

// converts a char/string character from hex representation to decimal representation
// eg. FF -> 255
int hexToDecimal(std::string hex_value) {
	std::stringstream ss;
	int decimal_value;
	ss << hex_value; // std::string hex_value
	ss >> std::hex >> decimal_value; //int decimal_value
	return decimal_value;
}

//Convert a two-character hex string to a single charracter
char hexToChar(const std::string& hex) {
	unsigned char value = 0;
	for (int i = 0; i < 2; ++i) {
		value <<= 4;
		if (hex[i] >= '0' && hex[i] <= '9') {
			value |= hex[i] - '0';
		}
		else if (hex[i] >= 'a' && hex[i] <= 'f') {
			value |= hex[i] - 'a' + 10;
		}
		else if (hex[i] >= 'A' && hex[i] <= 'F') {
			value |= hex[i] - 'A' + 10;
		}
	}
	return value;
}

// converts a string represented in hex to a string in ascii.
// eg. 74657374 -> test
std::string hexToString(const std::string& hex_data) {
	std::string asciiStr;
	asciiStr.reserve(hex_data.length() / 2);
	for (size_t i = 0; i < hex_data.length(); i += 2) {
		asciiStr += hexToChar(hex_data.substr(i, 2));
	}
	return asciiStr;
}


// converts an int value to hex using hex values 00 - 09
// eg. 4545 -> 04050405
std::string intToHexBytes(int length) {
	std::string temp = "";
	std::string intStr = std::to_string(length);
	for (auto i = 0; i < intStr.length(); i++)
	{
		temp += "0";
		temp += intStr[i];
	}
	return temp;
}

// convert hex string to bytes
std::vector<char> HexToBytes(std::string hex) {
	std::vector<char> bytes;

	for (unsigned int i = 0; i < hex.length(); i += 2) {
		std::string byteString = hex.substr(i, 2);
		char byte = (char)strtol(byteString.c_str(), NULL, 16);
		bytes.push_back(byte);
	}

	return bytes;
}

int writeToFile(std::string text, std::string log_type) {
	std::string prepend_text;
	if (log_type == "error") {
		prepend_text = "[ERROR]";
	}
	else if (log_type == "warning") {
		prepend_text = "[WARNING]";
	}
	else {
		prepend_text = "[DEBUG]";
	}

	auto const filename = "04DFAA.log";
	std::ofstream file1(filename, std::ios::app);

	if (file1.fail())
	{
		return EXIT_FAILURE;
	}
	std::string dt = currentDateTime();
	file1 << prepend_text << " " << dt << " - " << text << std::endl;
	return 0;
}
