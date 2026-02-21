#include "strUtil.h"
namespace strUtil
{
    std::string toLower(const std::string& str)
    {
        std::string result = str;
        for (char& c : result)
        {
            c = std::tolower(c);
        }
        return result;
    }

    std::string toUpper(const std::string& str)
    {
        std::string result = str;
        for (char& c : result)
        {
            c = std::toupper(c);
        }
        return result;
    }

    std::string trim(const std::string& str)
    {
        size_t start = str.find_first_not_of(" \t\n\r");
        size_t end = str.find_last_not_of(" \t\n\r");
        if (start == std::string::npos || end == std::string::npos)
        {
            return "";
        }
        return str.substr(start, end - start + 1);
    }

    bool startsWith(const std::string& str, const std::string& prefix)
    {
        return str.compare(0, prefix.size(), prefix) == 0;
    }

    bool endsWith(const std::string& str, const std::string& suffix)
    {
        if (suffix.size() > str.size())
        {
            return false;
        }
        return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }
}