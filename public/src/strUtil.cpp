#include "strUtil.h"
namespace strUtil
{
    std::string toLower(const std::string &str)
    {
        std::string result = str;
        for (char &c : result)
        {
            c = std::tolower(c);
        }
        return result;
    }

    std::string toUpper(const std::string &str)
    {
        std::string result = str;
        for (char &c : result)
        {
            c = std::toupper(c);
        }
        return result;
    }

    bool startsWith(const std::string &str, const std::string &prefix)
    {
        return str.compare(0, prefix.size(), prefix) == 0;
    }

    bool endsWith(const std::string &str, const std::string &suffix)
    {
        if (suffix.size() > str.size())
        {
            return false;
        }
        return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    char *ltrim(char *str, const char cc)
    {
        if (str == nullptr)
            return nullptr; // 如果传进来的是空地址，直接返回，防止程序崩溃。

        char *p = str;   // 指向字符串的首地址。
        while (*p == cc) // 遍历字符串，p将指向左边第一个不是cc的字符。
            p++;

        memmove(str, p, strlen(str) - (p - str) + 1); // 把结尾标志0也拷过来。

        return str;
    }

    std::string &ltrim(std::string &str, const char cc)
    {
        // 查找第一个不是 cc 的字符
        auto pos = str.find_first_not_of(cc);
        if (pos == std::string::npos)
        {
            str.clear(); // 全部都是 cc，直接清空字符串
        }
        else if (pos > 0)
        {
            str.erase(0, pos); // 删除前 pos 个字符
        }
        return str;
    }

    char *rtrim(char *str, const char cc)
    {
        if (str == nullptr)
            return nullptr; // 如果传进来的是空地址，直接返回，防止程序崩溃。

        char *p = str;   // 指向字符串的首地址。
        char *piscc = 0; // 右边全是字符cc的第一个位置。

        while (*p != 0) // 遍历字符串。
        {
            if (*p == cc && piscc == 0)
                piscc = p; // 记下字符cc的第一个位置。
            if (*p != cc)
                piscc = 0; // 只要当前字符不是cc，清空piscc。
            p++;
        }

        if (piscc != 0)
            *piscc = 0; // 把piscc位置的字符置为0，表示字符串已结束。

        return str;
    }

    std::string &rtrim(std::string &str, const char cc)
    {
        auto pos = str.find_last_not_of(cc); // 从字符串的右边查找第一个不是cc的字符的位置。

        if (pos != std::string::npos)
            str.erase(pos + 1); // 把pos之后的字符删掉。

        return str;
    }

    char *trim(char *str, const char cc)
    {
        ltrim(str, cc);
        rtrim(str, cc);

        return str;
    }

    std::string &trim(std::string &str, const char cc)
    {
        ltrim(str, cc);
        rtrim(str, cc);

        return str;
    }

    std::string trim2(const std::string &str)
    {
        size_t start = str.find_first_not_of(" \t\n\r");
        size_t end = str.find_last_not_of(" \t\n\r");
        if (start == std::string::npos || end == std::string::npos)
        {
            return "";
        }
        return str.substr(start, end - start + 1);
    }
}