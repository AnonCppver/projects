#include "strUtil.h"

namespace leef
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

    void toLower2(std::string &str)
    {
        for (char &c : str)
        {
            c = std::tolower(c);
        }
        return;
    }

    void toUpper2(std::string &str)
    {
        for (char &c : str)
        {
            c = std::toupper(c);
        }
        return;
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

    char *lTrim(char *str, const char cc)
    {
        if (str == nullptr)
            return nullptr; // 如果传进来的是空地址，直接返回，防止程序崩溃。

        char *p = str;   // 指向字符串的首地址。
        while (*p == cc) // 遍历字符串，p将指向左边第一个不是cc的字符。
            p++;

        memmove(str, p, strlen(str) - (p - str) + 1); // 把结尾标志0也拷过来。

        return str;
    }

    std::string &lTrim(std::string &str, const char cc)
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

    char *rTrim(char *str, const char cc)
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

    std::string &rTrim(std::string &str, const char cc)
    {
        auto pos = str.find_last_not_of(cc); // 从字符串的右边查找第一个不是cc的字符的位置。

        if (pos != std::string::npos)
            str.erase(pos + 1); // 把pos之后的字符删掉。

        return str;
    }

    char *trim(char *str, const char cc)
    {
        lTrim(str, cc);
        rTrim(str, cc);

        return str;
    }

    std::string &trim(std::string &str, const char cc)
    {
        lTrim(str, cc);
        rTrim(str, cc);

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

    bool replaceStr(std::string &str, const std::string &str1, const std::string &str2, const bool bloop)
    {
        // 如果原字符串str或旧的内容str1为空，没有意义，不执行替换。
        if ((str.length() == 0) || (str1.length() == 0))
            return false;

        // 如果bloop为true并且str2中包函了str1的内容，直接返回，因为会进入死循环，最终导致内存溢出。
        if ((bloop == true) && (str2.find(str1) != std::string::npos))
            return false;

        size_t pstart = 0; // 如果bloop==false，下一次执行替换的开始位置。
        size_t ppos = 0;   // 本次需要替换的位置。

        while (true)
        {
            if (bloop == true)
                ppos = str.find(str1); // 每次从字符串的最左边开始查找子串str1。
            else
                ppos = str.find(str1, pstart); // 从上次执行替换的位置后开始查找子串str1。

            if (ppos == std::string::npos)
                break; // 如果没有找到子串str1。

            str.replace(ppos, str1.length(), str2); // 把str1替换成str2。

            if (bloop == false)
                pstart = ppos + str2.length(); // 下一次执行替换的开始位置往右移动。
        }

        return true;
    }

    bool replaceStr(char *str, const std::string &str1, const std::string &str2, const bool bloop)
    {
        if (str == nullptr)
            return false;

        std::string strtemp(str);

        replaceStr(strtemp, str1, str2, bloop);

        strtemp.copy(str, strtemp.length());
        str[strtemp.length()] = 0; // string的copy函数不会给C风格字符串的结尾加0。

        return true;
    }

    char *pickNumber(const std::string &src, char *dest, const bool bsigned, const bool bdot)
    {
        if (dest == nullptr)
            return nullptr; // 判断空指针。

        std::string strtemp = pickNumber(src, bsigned, bdot);
        strtemp.copy(dest, strtemp.length());
        dest[strtemp.length()] = 0; // string的copy函数不会给C风格字符串的结尾加0。

        return dest;
    }

    std::string &pickNumber(const std::string &src, std::string &dest, const bool bsigned, const bool bdot)
    {
        // 为了支持src和dest是同一变量的情况，定义str临时变量。
        std::string str;

        for (char cc : src)
        {
            // 判断是否提取符号。
            if ((bsigned == true) && ((cc == '+') || (cc == '-')))
            {
                str.append(1, cc);
                continue;
            }

            // 判断是否提取小数点。
            if ((bdot == true) && (cc == '.'))
            {
                str.append(1, cc);
                continue;
            }

            // 提取数字。
            if (isdigit(cc))
                str.append(1, cc);
        }

        dest = str;

        return dest;
    }

    std::string pickNumber(const std::string &src, const bool bsigned, const bool bdot)
    {
        std::string dest;
        pickNumber(src, dest, bsigned, bdot);
        return dest;
    }

    bool matchStr(const std::string &str, const std::string &rules)
    {
        // 如果匹配规则表达式的内容是空的，返回false。
        if (rules.length() == 0)
            return false;

        // 如果如果匹配规则表达式的内容是"*"，直接返回true。
        if (rules == "*")
            return true;

        int ii, jj;
        size_t pos1, pos2;
        Splitter spltStr, spltSubstr;

        // 把字符串都转换成大写后再来比较
        std::string filename = toUpper(str);
        std::string matchstr = toUpper(rules);

        spltStr.str2vec(matchstr, ",");

        for (ii = 0; ii < spltStr.size(); ii++)
        {
            // 如果为空，就一定要跳过，否则就会被匹配上。
            if (spltStr[ii].empty() == true)
                continue;

            pos1 = pos2 = 0;
            spltSubstr.str2vec(spltStr[ii], "*");

            for (jj = 0; jj < spltSubstr.size(); jj++)
            {
                // 如果是文件名的首部
                if (jj == 0)
                    if (filename.substr(0, spltSubstr[jj].length()) != spltSubstr[jj])
                        break;

                // 如果是文件名的尾部
                if (jj == spltSubstr.size() - 1)
                    if (filename.find(spltSubstr[jj], filename.length() - spltSubstr[jj].length()) == std::string::npos)
                        break;

                pos2 = filename.find(spltSubstr[jj], pos1);

                if (pos2 == std::string::npos)
                    break;

                pos1 = pos2 + spltSubstr[jj].length();
            }

            if (jj == spltSubstr.size())
                return true;
        }

        return false;
    }

    Splitter::Splitter(const std::string &buffer, const std::string &sepstr, const bool bdelspace)
    {
        str2vec(buffer, sepstr, bdelspace);
    }

    // 把字符串拆分到m_vecTokens容器中。
    // buffer：待拆分的字符串。
    // sepstr：buffer字符串中字段内容的分隔符，注意，分隔符是字符串，如","、" "、"|"、"~!~"。
    // bdelspace：是否删除拆分后的字段内容前后的空格，true-删除；false-不删除，缺省不删除。
    void Splitter::str2vec(const std::string &buffer, const std::string &sepstr, const bool bdelspace)
    {
        // 清除所有的旧数据
        m_vecTokens.clear();

        size_t pos = 0;     // 每次从buffer中查找分隔符的起始位置。
        size_t pos1 = 0;    // 从pos的位置开始，查找下一个分隔符的位置。
        std::string substr; // 存放每次拆分出来的子串。

        while ((pos1 = buffer.find(sepstr, pos)) != std::string::npos) // 从pos的位置开始，查找下一个分隔符的位置。
        {
            substr = buffer.substr(pos, pos1 - pos); // 从buffer中截取子串。

            if (bdelspace == true)
                trim(substr); // 删除子串前后的空格。

            m_vecTokens.push_back(std::move(substr)); // 把子串放入m_vecTokens容器中，调用string类的移动构造函数。

            pos = pos1 + sepstr.length(); // 下次从buffer中查找分隔符的起始位置后移。
        }

        // 处理最后一个字段（最后一个分隔符之后的内容）。
        substr = buffer.substr(pos);

        if (bdelspace == true)
            trim(substr);

        m_vecTokens.push_back(std::move(substr));

        return;
    }

    bool Splitter::getvalue(const int ii, std::string &value, const int ilen) const
    {
        if (ii >= (int)m_vecTokens.size())
            return false;

        // 从xml中截取数据项的内容。
        // 视频中是以下代码：
        // value=m_vecTokens[ii];
        // 改为：
        int itmplen = m_vecTokens[ii].length();
        if ((ilen > 0) && (ilen < itmplen))
            itmplen = ilen;
        value = m_vecTokens[ii].substr(0, itmplen);

        return true;
    }

    bool Splitter::getvalue(const int ii, char *value, const int len) const
    {
        if ((ii >= (int)m_vecTokens.size()) || (value == nullptr))
            return false;

        if (len > 0)
            memset(value, 0, len + 1); // 调用者必须保证value的空间足够，否则这里会内存溢出。

        if ((m_vecTokens[ii].length() <= (unsigned int)len) || (len == 0))
        {
            m_vecTokens[ii].copy(value, m_vecTokens[ii].length());
            value[m_vecTokens[ii].length()] = 0; // string的copy函数不会给C风格字符串的结尾加0。
        }
        else
        {
            m_vecTokens[ii].copy(value, len);
            value[len] = 0;
        }

        return true;
    }

    bool Splitter::getvalue(const int ii, int &value) const
    {
        if (ii >= (int)m_vecTokens.size())
            return false;

        try
        {
            value = stoi(pickNumber(m_vecTokens[ii], true)); // stoi有异常，需要处理异常。
        }
        catch (const std::exception &e)
        {
            return false;
        }

        return true;
    }

    bool Splitter::getvalue(const int ii, unsigned int &value) const
    {
        if (ii >= (int)m_vecTokens.size())
            return false;

        try
        {
            value = stoi(pickNumber(m_vecTokens[ii])); // stoi有异常，需要处理异常。不提取符号 + -
        }
        catch (const std::exception &e)
        {
            return false;
        }

        return true;
    }

    bool Splitter::getvalue(const int ii, long &value) const
    {
        if (ii >= (int)m_vecTokens.size())
            return false;

        try
        {
            value = stol(pickNumber(m_vecTokens[ii], true)); // stol有异常，需要处理异常。
        }
        catch (const std::exception &e)
        {
            return false;
        }

        return true;
    }

    bool Splitter::getvalue(const int ii, unsigned long &value) const
    {
        if (ii >= (int)m_vecTokens.size())
            return false;

        try
        {
            value = stoul(pickNumber(m_vecTokens[ii])); // stoul有异常，需要处理异常。不提取符号 + -
        }
        catch (const std::exception &e)
        {
            return false;
        }

        return true;
    }

    bool Splitter::getvalue(const int ii, double &value) const
    {
        if (ii >= (int)m_vecTokens.size())
            return false;

        try
        {
            value = stod(pickNumber(m_vecTokens[ii], true, true)); // stod有异常，需要处理异常。提取符号和小数点。
        }
        catch (const std::exception &e)
        {
            return false;
        }

        return true;
    }

    bool Splitter::getvalue(const int ii, float &value) const
    {
        if (ii >= (int)m_vecTokens.size())
            return false;

        try
        {
            value = stof(pickNumber(m_vecTokens[ii], true, true)); // stof有异常，需要处理异常。提取符号和小数点。
        }
        catch (const std::exception &e)
        {
            return false;
        }

        return true;
    }

    bool Splitter::getvalue(const int ii, bool &value) const
    {
        if (ii >= (int)m_vecTokens.size())
            return false;

        std::string str = m_vecTokens[ii];
        toUpper2(str); // 转换为大写来判断。

        if (str == "TRUE")
            value = true;
        else
            value = false;

        return true;
    }

    Splitter::~Splitter()
    {
        m_vecTokens.clear();
    }

    bool matchstr(const std::string &str, const std::string &rules)
    {
        // 如果匹配规则表达式的内容是空的，返回false。
        if (rules.length() == 0)
            return false;

        // 如果如果匹配规则表达式的内容是"*"，直接返回true。
        if (rules == "*")
            return true;

        int ii, jj;
        int pos1, pos2;
        Splitter splitter, subSplitter;

        std::string filename = str;
        std::string matchstr = rules;

        // 把字符串都转换成大写后再来比较
        toUpper2(filename);
        toUpper2(matchstr);

        splitter.str2vec(matchstr, ",");

        for (ii = 0; ii < splitter.size(); ii++)
        {
            // 如果为空，就一定要跳过，否则就会被匹配上。
            if (splitter[ii].empty() == true)
                continue;

            pos1 = pos2 = 0;
            subSplitter.str2vec(splitter[ii], "*");

            for (jj = 0; jj < subSplitter.size(); jj++)
            {
                // 如果是文件名的首部
                if (jj == 0)
                    if (filename.substr(0, subSplitter[jj].length()) != subSplitter[jj])
                        break;

                // 如果是文件名的尾部
                if (jj == subSplitter.size() - 1)
                    if (filename.find(subSplitter[jj], filename.length() - subSplitter[jj].length()) == std::string::npos)
                        break;

                pos2 = filename.find(subSplitter[jj], pos1);

                if (pos2 == static_cast<int>(std::string::npos))
                    break;

                pos1 = pos2 + subSplitter[jj].length();
            }

            if (jj == subSplitter.size())
                return true;
        }

        return false;
    }
}