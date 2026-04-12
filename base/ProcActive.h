#ifndef _LEEF_BASE_PROC_ACTIVE_H
#define _LEEF_BASE_PROC_ACTIVE_H

#include <string>
#include <cstring>
#include <sys/shm.h>
#include <sys/sem.h>

#include "Logging.h"

namespace leef
{
    // 循环队列。
template <class TT, int MaxLength>
class Squeue
{
private:
    bool m_inited;              // 队列被初始化标志，true-已初始化；false-未初始化。
    TT   m_data[MaxLength];     // 用数组存储循环队列中的元素。
    int  m_head;                // 队列的头指针。
    int  m_tail;                // 队列的尾指针，指向队尾元素。
    int  m_length;              // 队列的实际长度。    
    Squeue(const Squeue &) = delete;             // 禁用拷贝构造函数。
    Squeue &operator=(const Squeue &) = delete;  // 禁用赋值函数。
public:

    Squeue() { init(); }  // 构造函数。

    // 循环队列的初始化操作。
    // 注意：如果用于共享内存的队列，不会调用构造函数，必须调用此函数初始化。
    void init()  
    { 
        if (m_inited!=true)               // 循环队列的初始化只能执行一次。
        { 
            m_head=0;                      // 头指针。
            m_tail=MaxLength-1;     // 为了方便写代码，初始化时，尾指针指向队列的最后一个位置。
            m_length=0;                   // 队列的实际长度。
            memset(m_data,0,sizeof(m_data));  // 数组元素清零。
            m_inited=true; 
        }
    }

    // 元素入队，返回值：false-失败；true-成功。
    bool push(const TT &ee)
    {
        if (full() == true)
        {
            LOG_INFO << "循环队列已满，入队失败。\n"; return false;
        }

        // 先移动队尾指针，然后再拷贝数据。
        m_tail=(m_tail+1)%MaxLength;  // 队尾指针后移。
        m_data[m_tail]=ee;
        m_length++;    

      return true;
    }

    // 求循环队列的长度，返回值：>=0-队列中元素的个数。
    int  size()                   
    {
        return m_length;    
    }

    // 判断循环队列是否为空，返回值：true-空，false-非空。
    bool empty()                    
    {
      if (m_length == 0) return true;    

      return false;
    }

    // 判断循环队列是否已满，返回值：true-已满，false-未满。
    bool full()
    {
        if (m_length == MaxLength) return true;    

      return false;
    }

    // 查看队头元素的值，元素不出队。
    TT& front()
    {
        return m_data[m_head];
    }

    // 元素出队，返回值：false-失败；true-成功。
    bool pop()
    {
        if (empty() == true) return false;

        m_head=(m_head+1)%MaxLength;  // 队列头指针后移。
        m_length--;    

        return true;
    }
};

// 信号量。
class Semp
{
private:
    union semun  // 用于信号量操作的共同体。
    {
      int val;
      struct semid_ds *buf;
      unsigned short  *arry;
    };

    int   m_semid;         // 信号量id（描述符）。

    // 如果把sem_flg设置为SEM_UNDO，操作系统将跟踪进程对信号量的修改情况，
    // 在全部修改过信号量的进程（正常或异常）终止后，操作系统将把信号量恢复为初始值。
    // 如果信号量用于互斥锁，设置为SEM_UNDO。
    // 如果信号量用于生产消费者模型，设置为0。
    short m_sem_flg;

    Semp(const Semp &) = delete;                      // 禁用拷贝构造函数。
    Semp &operator=(const Semp &) = delete;  // 禁用赋值函数。
public:
    Semp():m_semid(-1){}

    // 如果信号量已存在，获取信号量；如果信号量不存在，则创建它并初始化为value。
    // 如果用于互斥锁，value填1，sem_flg填SEM_UNDO。
    // 如果用于生产消费者模型，value填0，sem_flg填0。
    bool init(key_t key,unsigned short value=1,short sem_flg=SEM_UNDO);
    bool wait(short value=-1);    // 信号量的P操作，如果信号量的值是0，将阻塞等待，直到信号量的值大于0。
    bool post(short value=1);     // 信号量的V操作。
    int  getValue();                       // 获取信号量的值，成功返回信号量的值，失败返回-1。
    bool destroy();                       // 销毁信号量。
    ~Semp();
};

// 进程心跳信息的结构体。
struct ProcInfo
{
    int      pid=0;                      // 进程id。
    char   pname[51]={0};        // 进程名称，可以为空。
    int      timeout=0;              // 超时时间，单位：秒。
    time_t atime=0;                 // 最后一次心跳的时间，用整数表示。
    ProcInfo() = default;     // 有了自定义的构造函数，编译器将不提供默认构造函数，所以启用默认构造函数。
    ProcInfo(const int in_pid,const std::string & in_pname,const int in_timeout, const time_t in_atime)
                    :pid(in_pid),timeout(in_timeout),atime(in_atime) { strncpy(pname,in_pname.c_str(),50); }
};

// 以下几个宏用于进程的心跳。
#define MAXNUMP     1000     // 最大的进程数量。
#define SHMKEYP    0x5095     // 共享内存的key。
#define SEMKEYP     0x5095     // 信号量的key。

// 查看共享内存：  ipcs -m
// 删除共享内存：  ipcrm -m shmid
// 查看信号量：      ipcs -s
// 删除信号量：      ipcrm sem semid

// 进程心跳操作类。
class ProcActive
{
 private:
     int  m_shmid;                   // 共享内存的id。
     int  m_pos;                       // 当前进程在共享内存进程组中的位置。
     ProcInfo *m_shm;        // 指向共享内存的地址空间。

 public:
     ProcActive();  // 初始化成员变量。

     // 把当前进程的信息加入共享内存进程组中。
     bool addPinfo(const int timeout,const std::string &pname="");

     // 更新共享内存进程组中当前进程的心跳时间。
     bool updateTime();

     ~ProcActive();  // 从共享内存中删除当前进程的心跳记录。
};
}

#endif // _LEEF_BASE_PROC_ACTIVE_H