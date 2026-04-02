#ifndef _LEEF_FILEUTIL_CPP
#define _LEEF_FILEUTIL_CPP

#include <string>
#include <vector>

namespace leef::file
{
    // basic file operations
    bool isdir(const std::string &pathorfilename);
    bool isfile(const std::string &pathorfilename);
    bool exist(const std::string &pathorfilename);
    bool remove(const std::string &pathorfilename);
    bool move(const std::string &from, const std::string &to);
    bool copy(const std::string &from, const std::string &to);
    bool newdir(const std::string &dirname);
    bool rmdir(const std::string &dirname);
    size_t filesize(const std::string &pathorfilename);

    // 获取某目录及其子目录中的文件列表的类。
    class cdir
    {
    private:
        std::vector<std::string> m_filelist; // 存放文件列表的容器（绝对路径的文件名）。
        int m_pos;                           // 从文件列表m_filelist中已读取文件的位置。
        std::string m_fmt;                   // 文件时间格式，缺省"yyyymmddhh24miss"。

        cdir(const cdir &) = delete;            // 禁用拷贝构造函数。
        cdir &operator=(const cdir &) = delete; // 禁用赋值函数。
    public:
        // /project/public/_public.h
        std::string m_dirname;   // 目录名，例如：/project/public
        std::string m_filename;  // 文件名，不包括目录名，例如：_public.h
        std::string m_ffilename; // 绝对路径的文件，例如：/project/public/_public.h
        int m_filesize;          // 文件的大小，单位：字节。
        std::string m_mtime;     // 文件最后一次被修改的时间，即stat结构体的st_mtime成员。
        std::string m_ctime;     // 文件生成的时间，即stat结构体的st_ctime成员。
        std::string m_atime;     // 文件最后一次被访问的时间，即stat结构体的st_atime成员。

        cdir() : m_pos(0), m_fmt("yyyymmddhh24miss") {} // 构造函数。

        // 设置文件时间的格式，支持"yyyy-mm-dd hh24:mi:ss"和"yyyymmddhh24miss"两种，缺省是后者。
        void setfmt(const std::string &fmt);

        // 打开目录，获取目录中文件的列表，存放在m_filelist容器中。
        // dirname，目录名，采用绝对路径，如/tmp/root。
        // rules，文件名的匹配规则，不匹配的文件将被忽略。
        // maxfiles，本次获取文件的最大数量，缺省值为10000个，如果文件太多，可能消耗太多的内存。
        // bandchild，是否打开各级子目录，缺省值为false-不打开子目录。
        // bsort，是否按文件名排序，缺省值为false-不排序。
        // 返回值：true-成功，false-失败。
        bool opendir(const std::string &dirname, const std::string &rules, const int maxfiles = 10000, const bool bandchild = false, bool bsort = false);

    private:
        // 这是一个递归函数，被opendir()的调用，在cdir类的外部不需要调用它。
        bool _opendir(const std::string &dirname, const std::string &rules, const int maxfiles, const bool bandchild);

    public:
        // 从m_filelist容器中获取一条记录（文件名），同时获取该文件的大小、修改时间等信息。
        // 调用opendir方法时，m_filelist容器被清空，m_pos归零，每调用一次readdir方法m_pos加1。
        // 当m_pos小于m_filelist.size()，返回true，否则返回false。
        bool readdir();

        unsigned int size() { return m_filelist.size(); }

        ~cdir(); // 析构函数。
    };
}

#endif
