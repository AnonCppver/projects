#ifndef _PUBLIC_STRINGUTIL_H
#define _PUBLIC_STRINGUTIL_H

#include <string>
#include <string.h>
#include <ctype.h>
#include <vector>

namespace prj
{
    // 返回新string 或者原地替换
    std::string toLower(const std::string &str);
    std::string toUpper(const std::string &str);
    void toLower2(std::string &str);
    void toUpper2(std::string &str);

    bool startsWith(const std::string &str, const std::string &prefix);
    bool endsWith(const std::string &str, const std::string &suffix);

    char *lTrim(char *str, const char cc = ' ');
    std::string &lTrim(std::string &str, const char cc = ' ');

    char *rTrim(char *str, const char cc = ' ');
    std::string &rTrim(std::string &str, const char cc = ' ');

    char *trim(char *str, const char cc = ' ');
    std::string &trim(std::string &str, const char cc = ' ');
    std::string trim2(const std::string &str);

    // 字符串替换函数。
    // 在字符串str中，如果存在字符串str1，就替换为字符串str2。
    // str：待处理的字符串。
    // str1：旧的内容。
    // str2：新的内容。
    // bloop：是否循环执行替换。
    // 注意：
    // 1、如果str2比str1要长，替换后str会变长，所以必须保证str有足够的空间，否则内存会溢出（C++风格字符串不存在这个问题）。
    // 2、如果str2中包含了str1的内容，且bloop为true，这种做法存在逻辑错误，replacestr将什么也不做。
    // 3、如果str2为空，表示删除str中str1的内容。
    bool replaceStr(char *str, const std::string &str1, const std::string &str2, const bool bloop = false);
    bool replaceStr(std::string &str, const std::string &str1, const std::string &str2, const bool bloop = false);

    // 从一个字符串中提取出数字、符号和小数点，存放到另一个字符串中。
    // src：原字符串。
    // dest：目标字符串。
    // bsigned：是否提取符号（+和-），true-包括；false-不包括。
    // bdot：是否提取小数点（.），true-包括；false-不包括。
    // 注意：src和dest可以是同一个变量。
    char *pickNumber(const std::string &src, char *dest, const bool bsigned = false, const bool bdot = false);
    std::string &pickNumber(const std::string &src, std::string &dest, const bool bsigned = false, const bool bdot = false);
    std::string pickNumber(const std::string &src, const bool bsigned = false, const bool bdot = false);

    // 正则表达式，判断一个字符串是否匹配另一个字符串。
    // rules：匹配规则的表达式，用星号"*"代表任意字符，多个表达式之间用半角的逗号分隔，如"*.h,*.cpp"。
    // str参数不需要支持"*"，rules参数支持"*"；
    // 函数在判断str是否匹配rules的时候，会忽略字母的大小写。
    bool matchStr(const std::string &str, const std::string &rules);
    class Splitter
    {
    private:
        std::vector<std::string> m_vecTokens; // 存放拆分后的字段内容。

        Splitter(const Splitter &) = delete;            // 禁用拷贝构造函数。
        Splitter &operator=(const Splitter &) = delete; // 禁用赋值函数。
    public:
        Splitter() {} // 构造函数。
        Splitter(const std::string &buffer, const std::string &sepstr, const bool bdelspace = false);

        const std::string &operator[](int ii) const // 重载[]运算符，可以像访问数组一样访问m_vecTokens成员。
        {
            return m_vecTokens[ii];
        }

        // 把字符串拆分到m_vecTokens容器中。
        // buffer：待拆分的字符串。
        // sepstr：buffer中采用的分隔符，注意，sepstr参数的数据类型不是字符，是字符串，如","、" "、"|"、"~!~"。
        // bdelspace：拆分后是否删除字段内容前后的空格，true-删除；false-不删除，缺省不删除。
        void str2vec(const std::string &buffer, const std::string &sepstr, const bool bdelspace = false);

        // 获取拆分后字段的个数，即m_vecTokens容器的大小。
        int size() const { return m_vecTokens.size(); }

        // 从m_vecTokens容器获取字段内容。
        // ii：字段的顺序号，类似数组的下标，从0开始。
        // value：传入变量的地址，用于存放字段内容。
        // 返回值：true-成功；如果ii的取值超出了m_vecTokens容器的大小，返回失败。
        bool getvalue(const int ii, std::string &value, const int ilen = 0) const; // C++风格字符串。
        bool getvalue(const int ii, char *value, const int ilen = 0) const;        // C风格字符串，ilen缺省值为0-全部长度。
        bool getvalue(const int ii, int &value) const;                             // int整数。
        bool getvalue(const int ii, unsigned int &value) const;                    // unsigned int整数。
        bool getvalue(const int ii, long &value) const;                            // long整数。
        bool getvalue(const int ii, unsigned long &value) const;                   // unsigned long整数。
        bool getvalue(const int ii, double &value) const;                          // 双精度double。
        bool getvalue(const int ii, float &value) const;                           // 单精度float。
        bool getvalue(const int ii, bool &value) const;                            // bool型。

        ~Splitter(); // 析构函数。
    };

    // C++格式化输出函数模板。
    template <typename... Args>
    bool sformat(std::string &str, const char *fmt, Args... args)
    {
        int len = snprintf(nullptr, 0, fmt, args...); // 得到格式化输出后字符串的总长度。
        if (len < 0)
            return false; // 如果调用snprintf失败，返回-1。
        if (len == 0)
        {
            str.clear();
            return true;
        } // 如果调用snprintf返回0，表示格式化输出的内容为空。

        str.resize(len);
        snprintf(&str[0], len + 1, fmt, args...);
        return true;
    }
    template <typename... Args>
    std::string sformat(const char *fmt, Args... args)
    {
        std::string str;

        int len = snprintf(nullptr, 0, fmt, args...); // 得到格式化后字符串的长度。
        if (len <= 0)
            return str; // 如果调用snprintf失败，返回-1;如果调用snprintf返回0，表示格式化输出的内容为空。

        str.resize(len);
        snprintf(&str[0], len + 1, fmt, args...);
        return str;
    }
}

#endif // _PUBLIC_STRINGUTIL_H