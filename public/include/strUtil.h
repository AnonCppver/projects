#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <string>

namespace strUtil
{
    std::string toLower(const std::string& str);
    std::string toUpper(const std::string& str);
    std::string trim(const std::string& str);
    bool startsWith(const std::string& str, const std::string& prefix);
    bool endsWith(const std::string& str, const std::string& suffix);
}

#endif // STRINGUTIL_H