#include "fileUtil.h"
#include "timeUtil.h"
#include "strUtil.h"

#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fstream>
#include <string.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>

namespace leef::file
{
    bool isdir(const std::string &pathorfilename)
    {
        struct stat st;
        if (stat(pathorfilename.c_str(), &st) == 0)
        {
            return S_ISDIR(st.st_mode);
        }
        return false;
    }
    bool isfile(const std::string &pathorfilename)
    {
        struct stat st;
        if (stat(pathorfilename.c_str(), &st) == 0)
        {
            return S_ISREG(st.st_mode);
        }
        return false;
    }
    bool exist(const std::string &pathorfilename)
    {
        struct stat st;
        return stat(pathorfilename.c_str(), &st) == 0;
    }
    bool remove(const std::string &pathorfilename)
    {
        return unlink(pathorfilename.c_str()) == 0;
    }
    bool move(const std::string &from, const std::string &to)
    {
        // 如果原文件不存在，直接返回失败。
        if (access(from.c_str(), R_OK) != 0)
            return false;

        // 创建目标文件的目录。
        if (newdir(to) == false)
            return false;

        // 调用操作系统的库函数rename重命名文件。 mv
        if (rename(from.c_str(), to.c_str()) == 0)
            return true;

        return false;
    }
    bool copy(const std::string &from, const std::string &to)
    {
        int src = open(from.c_str(), O_RDONLY);
        int dst = open(to.c_str(), O_WRONLY | O_CREAT, 0755);

        off_t offset = 0;
        size_t count = filesize(from);
        size_t bytesCopied = 0;
        bytesCopied = sendfile(dst, src, &offset, count);

        close(src);
        close(dst);

        return bytesCopied == count;
    }
    bool newdir(const std::string &pathorfilename)
    {
        // /tmp/aaa/bbb/ccc/ddd    /tmp    /tmp/aaa    /tmp/aaa/bbb    /tmp/aaa/bbb/ccc

        // 检查目录是否存在，如果不存在，逐级创建子目录
        int pos = 1; // 不要从0开始，0是根目录/。

        while (true)
        {
            size_t pos1 = pathorfilename.find('/', pos);
            if (pos1 == std::string::npos)
                break;

            std::string strpathname = pathorfilename.substr(0, pos1); // 截取目录。

            pos = pos1 + 1;                             // 位置后移。
            if (access(strpathname.c_str(), F_OK) != 0) // 如果目录不存在，创建它。
            {
                // 0755是八进制，不要写成755。
                if (mkdir(strpathname.c_str(), 0755) != 0)
                    return false; // 如果目录不存在，创建它。
            }
        }

        // 如果pathorfilename不是文件，是目录，还需要创建最后一级子目录。
        if (isfile(pathorfilename))
        {
            if (access(pathorfilename.c_str(), F_OK) != 0)
            {
                if (mkdir(pathorfilename.c_str(), 0755) != 0)
                    return false;
            }
        }

        return true;
    }
    bool rmdir(const std::string &dirname)
    {
        return ::rmdir(dirname.c_str()) == 0;
    }
    size_t filesize(const std::string &pathorfilename)
    {
        struct stat st;
        if (stat(pathorfilename.c_str(), &st) == 0)
        {
            return st.st_size;
        }
        return 0;
    }

    void Dir::setfmt(const std::string &fmt)
    {
        m_fmt = fmt;
    }

    bool Dir::opendir(const std::string &dirname, const std::string &rules, const int maxfiles, const bool bandchild, bool bsort)
    {
        m_filelist.clear(); // 清空文件列表容器。
        m_pos = 0;          // 从文件列表中已读取文件的位置归0。

        // 如果目录不存在，创建它。
        if (newdir(dirname) == false)
            return false;

        // 打开目录，获取目录中的文件列表，存放在m_filelist容器中。
        bool ret = _opendir(dirname, rules, maxfiles, bandchild);

        if (bsort == true) // 对文件列表排序。
        {
            sort(m_filelist.begin(), m_filelist.end());
        }

        return ret;
    }

    // 这是一个递归函数，在opendir()中调用，cdir类的外部不需要调用它。
    bool Dir::_opendir(const std::string &dirname, const std::string &rules, const int maxfiles, const bool bandchild)
    {
        DIR *dir; // 目录指针。

        // 打开目录。
        if ((dir = ::opendir(dirname.c_str())) == nullptr)
            return false; // opendir与库函数重名，需要加::

        std::string strffilename;  // 全路径的文件名。
        struct dirent *stdir; // 存放从目录中读取的内容。

        // 用循环读取目录的内容，将得到目录中的文件名和子目录。
        while ((stdir = ::readdir(dir)) != 0) // readdir与库函数重名，需要加::
        {
            // 判断容器中的文件数量是否超出maxfiles参数。
            if (static_cast<int>(m_filelist.size()) >= maxfiles)
                break;

            // 文件名以"."打头的文件不处理。.是当前目录，..是上一级目录，其它以.打头的都是特殊目录和文件。
            if (stdir->d_name[0] == '.')
                continue;

            // 拼接全路径的文件名。
            strffilename = dirname + '/' + stdir->d_name;

            // 如果是目录，处理各级子目录。
            if (stdir->d_type == 4)
            {
                if (bandchild == true) // 打开各级子目录。
                {
                    if (_opendir(strffilename, rules, maxfiles, bandchild) == false) // 递归调用_opendir函数。
                    {
                        closedir(dir);
                        return false;
                    }
                }
            }

            // 如果是普通文件，放入容器中。
            if (stdir->d_type == 8)
            {
                // 把能匹配上的文件放入m_filelist容器中。
                if (matchstr(stdir->d_name, rules) == false)
                    continue;

                m_filelist.push_back(std::move(strffilename));
            }
        }

        closedir(dir); // 关闭目录。

        return true;
    }

    bool Dir::readdir()
    {
        // 如果已读完，清空容器
        if (m_pos >= static_cast<int>(m_filelist.size()))
        {
            m_pos = 0;
            m_filelist.clear();
            return false;
        }

        // 文件全名，包括路径
        m_ffilename = m_filelist[m_pos];

        // 从绝对路径的文件名中解析出目录名和文件名。
        int pp = m_ffilename.find_last_of("/");
        m_dirname = m_ffilename.substr(0, pp);
        m_filename = m_ffilename.substr(pp + 1);

        // 获取文件的信息。
        struct stat st_filestat;
        stat(m_ffilename.c_str(), &st_filestat);
        m_filesize = st_filestat.st_size;                  // 文件大小。
        m_mtime = time2str(st_filestat.st_mtime, m_fmt); // 文件最后一次被修改的时间。
        m_ctime = time2str(st_filestat.st_ctime, m_fmt); // 文件生成的时间。
        m_atime = time2str(st_filestat.st_atime, m_fmt); // 文件最后一次被访问的时间。

        m_pos++; // 已读取文件的位置后移。

        return true;
    }

    Dir::~Dir()
    {
        m_filelist.clear();
    }
}