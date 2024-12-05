#pragma once

// Defined in src/common/crypto/djb2.cpp
namespace djb2 {

/*
 * djb2 (defined in src/common/crypto/djb2.cpp):
 *      About:
 *          Calculates the DJB2 hash of the input. Expects input to be a null-terminated string
 *      Result:
 *          Returns the DJB2 hash of the input as an unsigned long.
 */
unsigned long djb2(const char* input);

/*
 * djb2_case_insensitive (defined in src/common/crypto/djb2.cpp):
 *      About:
 *          Calculates the case-insensitive DJB2 hash of the input. Expects input to be a null-terminated string.
 *          For case-insensitivity, any capitalized A-Z characters are converted to their lowercase equivalent.
 *      Result:
 *          Returns the case-insensitive DJB2 hash of the input as an unsigned long.
 */
unsigned long djb2_case_insensitive(const char* input);

/*
 * StrEqDJB2 (defined in src/common/crypto/djb2.cpp):
 *      About:
 *          Checks if the DJB2 hash of the input is equal to the expected hash and that the 
 *          input's first character matches the expected first character (to minimize collisions)
 *      Result:
 *          True if the inputs are equal according to the specified conditions, false otherwise
 */
bool StrEqDJB2(const char* input, unsigned long expected, char first);

} // namespace