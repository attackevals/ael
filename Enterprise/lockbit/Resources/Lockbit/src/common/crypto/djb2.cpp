#include "crypto/djb2.hpp"

namespace djb2 {

/*
 * djb2:
 *      About:
 *          Calculates the DJB2 hash of the input. Expects input to be a null-terminated string
 *      Result:
 *          Returns the DJB2 hash of the input as an unsigned long.
 *      Other References:
 *          http://www.cse.yorku.ca/~oz/hash.html
 */
unsigned long djb2(const char* input) {
    unsigned long hash = 5381;
    int c;
    while (c = *input++) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

/*
 * djb2_case_insensitive:
 *      About:
 *          Calculates the case-insensitive DJB2 hash of the input. Expects input to be a null-terminated string.
 *          For case-insensitivity, any capitalized A-Z characters are converted to their lowercase equivalent.
 *      Result:
 *          Returns the case-insensitive DJB2 hash of the input as an unsigned long.
 *      Other References:
 *          http://www.cse.yorku.ca/~oz/hash.html
 */
unsigned long djb2_case_insensitive(const char* input) {
    unsigned long hash = 5381;
    int c;
    while (c = *input++) {
        if (c <= 90 && c >= 65) {
            c += 32; // convert to lowercase.
        }
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

// Checks if the DJB2 hash of the input is equal to the expected hash and that the 
// input's first character matches the expected first character (to minimize collisions)
bool StrEqDJB2(const char* input, unsigned long expected, char first) {
    return djb2::djb2(input) == expected && input[0] == first;
}

} // namespace
