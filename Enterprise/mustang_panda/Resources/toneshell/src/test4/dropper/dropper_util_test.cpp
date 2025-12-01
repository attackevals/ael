#include "dropper_util.hpp"
#include <gtest/gtest.h>

TEST(DropperUtilTest, TestConcatToParent) {
    wchar_t buf[40];

    // Test 1 backslash
    wcscpy(buf, L"C:\\mypath.txt");
    ConcatToParent(buf, 40, L"toappend");
    EXPECT_STREQ(buf, L"C:\\toappend");

    // Test 2 backslashes
    wcscpy(buf, L"C:\\mypath\\two.txt");
    ConcatToParent(buf, 40, L"append\\with\\backslashes");
    EXPECT_STREQ(buf, L"C:\\append\\with\\backslashes");

    // Test 3 backslashes
    wcscpy(buf, L"C:\\mypath\\a\\three.pdf");
    ConcatToParent(buf, 40, L"dir\\leaf");
    EXPECT_STREQ(buf, L"C:\\mypath\\dir\\leaf");

    // Test truncate
    wcscpy(buf, L"C:\\somewhat\\long\\base\\path");
    ConcatToParent(buf, 40, L"somewhat\\long\\item\\toappend");
    EXPECT_STREQ(buf, L"C:\\somewhat\\long\\somewhat\\long\\item\\toa");
}
