#include <vector>
#include <stdexcept>
#include <string>
#include "argparser.hpp"
#include <gtest/gtest.h>


TEST(ArgParser, TestConstructor) {
    char* argv[] = {
        (char*)"main.exe", 
        (char*)"-f", 
        (char*)"-s", 
        (char*)"value", 
        NULL
    };
    ArgParser args(4, argv);
    std::vector<std::string> want = {"-f", "-s", "value"};
    ASSERT_EQ(args.GetArgTokens(), want);
}

TEST(ArgParser, TestValidArgs) {
    char* argv[] = {
        (char*)"main.exe", 
        (char*)"-f", 
        (char*)"-s", 
        (char*)"value", 
        NULL
    };
    ArgParser args(4, argv);
    args.SetFlag("-f");
    args.SetStringArg("-s");
    args.Parse();
    EXPECT_TRUE(args.GetFlag("f"));
    EXPECT_TRUE(args.GetFlag("-f"));
    EXPECT_EQ(args.GetStringArg("s"), std::string("value"));
    EXPECT_EQ(args.GetStringArg("-s"), std::string("value"));
}

TEST(ArgParser, TestBoolFlag) {
    char* argv[] = {
        (char*)"main.exe", 
        NULL
    };
    ArgParser args(1, argv);
    args.SetFlag("-f");
    args.SetFlag("-s");
    args.Parse();
    EXPECT_FALSE(args.GetFlag("f"));
    EXPECT_FALSE(args.GetFlag("-f"));
    EXPECT_FALSE(args.GetFlag("s"));
    EXPECT_FALSE(args.GetFlag("-s"));

    char* argv2[] = {
        (char*)"main.exe", 
        (char*)"-f", 
        NULL
    };
    ArgParser args2(2, argv2);
    args2.SetFlag("-f");
    args2.SetFlag("-s");
    args2.Parse();
    EXPECT_TRUE(args2.GetFlag("f"));
    EXPECT_TRUE(args2.GetFlag("-f"));
    EXPECT_FALSE(args2.GetFlag("s"));
    EXPECT_FALSE(args2.GetFlag("-s"));
}

TEST(ArgParser, TestStringArgs) {
    char* argv[] = {
        (char*)"main.exe", 
        NULL
    };
    ArgParser args(1, argv);
    args.SetStringArg("-a");
    args.SetStringArg("-b");
    args.Parse();
    EXPECT_EQ(args.GetStringArg("a"), std::string(""));
    EXPECT_EQ(args.GetStringArg("-a"), std::string(""));
    EXPECT_EQ(args.GetStringArg("b"), std::string(""));
    EXPECT_EQ(args.GetStringArg("-b"), std::string(""));

    char* argv2[] = {
        (char*)"main.exe", 
        (char*)"-b", 
        (char*)"value", 
        NULL
    };
    ArgParser args2(3, argv2);
    args2.SetStringArg("-a");
    args2.SetStringArg("-b");
    args2.Parse();
    EXPECT_EQ(args2.GetStringArg("a"), std::string(""));
    EXPECT_EQ(args2.GetStringArg("-a"), std::string(""));
    EXPECT_EQ(args2.GetStringArg("b"), std::string("value"));
    EXPECT_EQ(args2.GetStringArg("-b"), std::string("value"));

    char* argv3[] = {
        (char*)"main.exe", 
        (char*)"-b", 
        (char*)"value",
        (char*)"-a", 
        (char*)"value2",
        NULL
    };
    ArgParser args3(5, argv3);
    args3.SetStringArg("-a");
    args3.SetStringArg("-b");
    args3.Parse();
    EXPECT_EQ(args3.GetStringArg("a"), std::string("value2"));
    EXPECT_EQ(args3.GetStringArg("-a"), std::string("value2"));
    EXPECT_EQ(args3.GetStringArg("b"), std::string("value"));
    EXPECT_EQ(args3.GetStringArg("-b"), std::string("value"));
}

TEST(ArgParser, TestSingleStandaloneArg) {
    char* argv[] = {
        (char*)"main.exe", 
        (char*)"-s", 
        (char*)"value", 
        (char*)"-f", 
        (char*)"standalone", 
        NULL
    };
    ArgParser args(5, argv);
    args.SetFlag("-f");
    args.SetStringArg("-s");
    args.AllowStandaloneValues();
    args.Parse();
    EXPECT_TRUE(args.GetFlag("f"));
    EXPECT_TRUE(args.GetFlag("-f"));
    EXPECT_EQ(args.GetStringArg("s"), std::string("value"));
    EXPECT_EQ(args.GetStringArg("-s"), std::string("value"));
    EXPECT_EQ(args.GetStandaloneValues(), std::vector<std::string>{"standalone"});
}

TEST(ArgParser, TestMultipleStandaloneArgs) {
    char* argv[] = {
        (char*)"main.exe", 
        (char*)"-s", 
        (char*)"value", 
        (char*)"-f", 
        (char*)"standalone", 
        (char*)"standalone2",
        (char*)"standalone3", 
        NULL
    };
    ArgParser args(7, argv);
    args.SetFlag("-f");
    args.SetStringArg("-s");
    args.AllowStandaloneValues();
    args.Parse();
    EXPECT_TRUE(args.GetFlag("f"));
    EXPECT_TRUE(args.GetFlag("-f"));
    EXPECT_EQ(args.GetStringArg("s"), std::string("value"));
    EXPECT_EQ(args.GetStringArg("-s"), std::string("value"));
    std::vector<std::string> want = {"standalone", "standalone2", "standalone3"};
    EXPECT_EQ(args.GetStandaloneValues(), want);

    char* argv2[] = {
        (char*)"main.exe", 
        (char*)"standalone", 
        (char*)"standalone2",
        (char*)"standalone3", 
        NULL
    };
    ArgParser args2(4, argv2);
    args2.SetFlag("-f");
    args2.SetStringArg("-s");
    args2.AllowStandaloneValues();
    args2.Parse();
    EXPECT_FALSE(args2.GetFlag("f"));
    EXPECT_FALSE(args2.GetFlag("-f"));
    EXPECT_EQ(args2.GetStringArg("s"), std::string(""));
    EXPECT_EQ(args2.GetStringArg("-s"), std::string(""));
    EXPECT_EQ(args2.GetStandaloneValues(), want);
}

TEST(ArgParser, TestSetDuplicateArgs) {
    char* argv[] = {
        (char*)"main.exe", 
        NULL
    };
    ArgParser args(1, argv);
    args.SetFlag("-f");
    args.SetStringArg("-s");
    EXPECT_THROW({
        try {
            args.SetFlag("-f");
        } catch (const std::invalid_argument& e) {
            EXPECT_STREQ("Cannot set option -f more than once", e.what());
            throw;
        }
    }, std::invalid_argument);
    EXPECT_THROW({
        try {
           args.SetStringArg("-s");
        } catch (const std::invalid_argument& e) {
            EXPECT_STREQ("Cannot set option -s more than once", e.what());
            throw;
        }
    }, std::invalid_argument);
}

TEST(ArgParser, TestSetInvalidOption) {
    char* argv[] = {
        (char*)"main.exe", 
        NULL
    };
    ArgParser args(1, argv);
    EXPECT_THROW({
        try {
            args.SetFlag("f");
        } catch (const std::invalid_argument& e) {
            EXPECT_STREQ("Option must start with -", e.what());
            throw;
        }
    }, std::invalid_argument);
    EXPECT_THROW({
        try {
            args.SetStringArg("arg");
        } catch (const std::invalid_argument& e) {
            EXPECT_STREQ("Option must start with -", e.what());
            throw;
        }
    }, std::invalid_argument);
}

TEST(ArgParser, TestMissingArgValue) {
    char* argv[] = {
        (char*)"main.exe", 
        (char*)"-s",  
        (char*)"-f", 
        NULL
    };
    ArgParser args(3, argv);
    args.SetFlag("-f");
    args.SetStringArg("-s");
    EXPECT_THROW({
        try {
            args.Parse();
        } catch (const std::invalid_argument& e) {
            EXPECT_STREQ("Value not provided for option -s", e.what());
            throw;
        }
    }, std::invalid_argument);

    char* argv2[] = {
        (char*)"main.exe", 
        (char*)"standalone", 
        (char*)"standalone2",
        (char*)"standalone3", 
        (char*)"-d",
        (char*)"-f",
        NULL
    };
    ArgParser args2(6, argv2);
    args2.SetFlag("-f");
    args2.SetStringArg("-d");
    args2.AllowStandaloneValues();
    EXPECT_THROW({
        try {
            args2.Parse();
        } catch (const std::invalid_argument& e) {
            EXPECT_STREQ("Value not provided for option -d", e.what());
            throw;
        }
    }, std::invalid_argument);

    char* argv3[] = {
        (char*)"main.exe", 
        (char*)"standalone", 
        (char*)"standalone2",
        (char*)"standalone3", 
        (char*)"-d",

        NULL
    };
    ArgParser args3(5, argv3);
    args3.SetFlag("-f");
    args3.SetStringArg("-d");
    args3.AllowStandaloneValues();
    EXPECT_THROW({
        try {
            args3.Parse();
        } catch (const std::invalid_argument& e) {
            EXPECT_STREQ("Value not provided for option -d", e.what());
            throw;
        }
    }, std::invalid_argument);
}

TEST(ArgParser, TestParseDuplicateArgs) {
    char* argv[] = {
        (char*)"main.exe", 
        (char*)"-f",
        (char*)"-s", 
        (char*)"test", 
        (char*)"-f", 
        NULL
    };
    ArgParser args(5, argv);
    args.SetFlag("-f");
    args.SetStringArg("-s");
    EXPECT_THROW({
        try {
            args.Parse();
        } catch (const std::invalid_argument& e) {
            EXPECT_STREQ("Cannot provide option -f more than once", e.what());
            throw;
        }
    }, std::invalid_argument);

    char* argv2[] = {
        (char*)"main.exe", 
        (char*)"-f",
        (char*)"-s", 
        (char*)"test", 
        (char*)"-s", 
        (char*)"test2", 
        NULL
    };
    ArgParser args2(6, argv2);
    args2.SetFlag("-f");
    args2.SetStringArg("-s");
    EXPECT_THROW({
        try {
            args2.Parse();
        } catch (const std::invalid_argument& e) {
            EXPECT_STREQ("Cannot provide option -s more than once", e.what());
            throw;
        }
    }, std::invalid_argument);
}

TEST(ArgParser, TestStandaloneNotllowed) {
    char* argv[] = {
        (char*)"main.exe", 
        (char*)"bad", 
        (char*)"-f", 
        NULL
    };
    ArgParser args(3, argv);
    args.SetFlag("-f");
    args.SetStringArg("-s");
    EXPECT_THROW({
        try {
            args.Parse();
        } catch (const std::invalid_argument& e) {
            EXPECT_STREQ("Standalone values not allowed. Was given: bad", e.what());
            throw;
        }
    }, std::invalid_argument);

    char* argv2[] = {
        (char*)"main.exe", 
        (char*)"-s",
        (char*)"good", 
        (char*)"bad2", 
        NULL
    };
    ArgParser args2(4, argv2);
    args2.SetFlag("-f");
    args2.SetStringArg("-s");
    EXPECT_THROW({
        try {
            args2.Parse();
        } catch (const std::invalid_argument& e) {
            EXPECT_STREQ("Standalone values not allowed. Was given: bad2", e.what());
            throw;
        }
    }, std::invalid_argument);
}

TEST(ArgParser, TestUnrecognizedOption) {
    char* argv[] = {
        (char*)"main.exe", 
        (char*)"-x",
        NULL
    };
    ArgParser args(2, argv);
    args.SetFlag("-f");
    args.SetStringArg("-s");
    EXPECT_THROW({
        try {
            args.Parse();
        } catch (const std::invalid_argument& e) {
            EXPECT_STREQ("Unrecognized option -x", e.what());
            throw;
        }
    }, std::invalid_argument);

    char* argv2[] = {
        (char*)"main.exe", 
        (char*)"-s", 
        (char*)"test", 
        (char*)"-x",
        NULL
    };
    ArgParser args2(4, argv2);
    args2.SetFlag("-f");
    args2.SetStringArg("-s");
    EXPECT_THROW({
        try {
            args2.Parse();
        } catch (const std::invalid_argument& e) {
            EXPECT_STREQ("Unrecognized option -x", e.what());
            throw;
        }
    }, std::invalid_argument);
}