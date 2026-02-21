#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <string>
#include <string.h>

namespace strUtil
{
    std::string toLower(const std::string &str);
    std::string toUpper(const std::string &str);

    bool startsWith(const std::string &str, const std::string &prefix);
    bool endsWith(const std::string &str, const std::string &suffix);

    char *ltrim(char *str, const char cc = ' ');
    std::string &ltrim(std::string &str, const char cc = ' ');

    char *rtrim(char *str, const char cc = ' ');
    std::string &rtrim(std::string &str, const char cc = ' ');

    char *trim(char *str, const char cc = ' ');
    std::string &trim(std::string &str, const char cc = ' ');
    std::string trim2(const std::string &str);
}

#endif // STRINGUTIL_H