#include "ProcActive.h"

namespace leef
{
    ProcActive::ProcActive()
    {
        m_shmid = 0;
        m_pos = -1;
        m_shm = 0;
    }

    // 把当前进程的信息加入共享内存进程组中。
    bool ProcActive::addPinfo(const int timeout, const std::string &pname)
    {
        if (m_pos != -1)
            return true;

        // 创建/获取共享内存，键值为SHMKEYP，大小为MAXNUMP个st_procinfo结构体的大小。
        if ((m_shmid = shmget((key_t)SHMKEYP, MAXNUMP * sizeof(struct ProcInfo), 0666 | IPC_CREAT)) == -1)
        {
            LOG_ERROR << "创建/获取共享内存(" << SHMKEYP << ")失败。";
            return false;
        }

        // 将共享内存连接到当前进程的地址空间。
        m_shm = (struct ProcInfo *)shmat(m_shmid, 0, 0);

        /*
        struct ProcInfo procinfo;    // 当前进程心跳信息的结构体。
        memset(&procinfo,0,sizeof(procinfo));
        procinfo.pid=getpid();            // 当前进程号。
        procinfo.timeout=timeout;         // 超时时间。
        procinfo.atime=time(0);           // 当前时间。
        strncpy(procinfo.pname,pname.c_str(),50); // 进程名。
        */
        ProcInfo procinfo(getpid(), pname.c_str(), timeout, time(0)); // 当前进程心跳信息的结构体。

        // 进程id是循环使用的，如果曾经有一个进程异常退出，没有清理自己的心跳信息，
        // 它的进程信息将残留在共享内存中，不巧的是，如果当前进程重用了它的id，
        // 守护进程检查到残留进程的信息时，会向进程id发送退出信号，将误杀当前进程。
        // 所以，如果共享内存中已存在当前进程编号，一定是其它进程残留的信息，当前进程应该重用这个位置。
        for (int ii = 0; ii < MAXNUMP; ii++)
        {
            if ((m_shm + ii)->pid == procinfo.pid)
            {
                m_pos = ii;
                break;
            }
        }

        Semp semp; // 用于给共享内存加锁的信号量id。

        if (semp.init(SEMKEYP) == false) // 初始化信号量。
        {
            LOG_ERROR << "创建/获取信号量(" << SEMKEYP << ")失败。";
            return false;
        }

        semp.wait(); // 给共享内存上锁。

        // 如果m_pos==-1，表示共享内存的进程组中不存在当前进程编号，那就找一个空位置。
        if (m_pos == -1)
        {
            for (int ii = 0; ii < MAXNUMP; ii++)
                if ((m_shm + ii)->pid == 0)
                {
                    m_pos = ii;
                    break;
                }
        }

        // 如果m_pos==-1，表示没找到空位置，说明共享内存的空间已用完。
        if (m_pos == -1)
        {
            LOG_WARN << "共享内存空间已用完。";

            semp.post(); // 解锁。

            return false;
        }

        // 把当前进程的心跳信息存入共享内存的进程组中。
        memcpy(m_shm + m_pos, &procinfo, sizeof(struct ProcInfo));

        semp.post(); // 解锁。

        return true;
    }

    // 更新共享内存进程组中当前进程的心跳时间。
    bool ProcActive::updateTime()
    {
        if (m_pos == -1)
            return false;

        (m_shm + m_pos)->atime = time(0);

        return true;
    }

    ProcActive::~ProcActive()
    {
        // 把当前进程从共享内存的进程组中移去。
        // warning: ‘void* memset(void*, int, size_t)’ clearing an object of non-trivial type ‘struct leef::ProcInfo’; 
        //use assignment or value-initialization instead [-Wclass-memaccess]
        if (m_pos != -1)
            m_shm[m_pos] = ProcInfo{};

        // 把共享内存从当前进程中分离。
        if (m_shm != 0)
            shmdt(m_shm);
    }

    // 如果信号量已存在，获取信号量；如果信号量不存在，则创建它并初始化为value。
    // 如果用于互斥锁，value填1，sem_flg填SEM_UNDO。
    // 如果用于生产消费者模型，value填0，sem_flg填0。
    bool Semp::init(key_t key, unsigned short value, short sem_flg)
    {
        if (m_semid != -1)
            return false; // 如果已经初始化了，不必再次初始化。

        m_sem_flg = sem_flg;

        // 信号量的初始化不能直接用semget(key,1,0666|IPC_CREAT)
        // 因为信号量创建后，初始值是0，如果用于互斥锁，需要把它的初始值设置为1，
        // 而获取信号量则不需要设置初始值，所以，创建信号量和获取信号量的流程不同。

        // 信号量的初始化分三个步骤：
        // 1）获取信号量，如果成功，函数返回。
        // 2）如果失败，则创建信号量。
        // 3) 设置信号量的初始值。

        // 获取信号量。
        if ((m_semid = semget(key, 1, 0666)) == -1)
        {
            // 如果信号量不存在，创建它。
            if (errno == ENOENT)
            {
                // 用IPC_EXCL标志确保只有一个进程创建并初始化信号量，其它进程只能获取。
                if ((m_semid = semget(key, 1, 0666 | IPC_CREAT | IPC_EXCL)) == -1)
                {
                    if (errno == EEXIST) // 如果错误代码是信号量已存在，则再次获取信号量。
                    {
                        if ((m_semid = semget(key, 1, 0666)) == -1)
                        {
                            LOG_ERROR << "init 1 semget()";
                            return false;
                        }
                        return true;
                    }
                    else // 如果是其它错误，返回失败。
                    {
                        LOG_ERROR << "init 2 semget()";
                        return false;
                    }
                }

                // 信号量创建成功后，还需要把它初始化成value。
                union semun sem_union;
                sem_union.val = value; // 设置信号量的初始值。
                if (semctl(m_semid, 0, SETVAL, sem_union) < 0)
                {
                    LOG_ERROR << "init semctl()";
                    return false;
                }
            }
            else
            {
                LOG_ERROR << "init 3 semget()";
                return false;
            }
        }

        return true;
    }

    // 信号量的P操作（把信号量的值减value），如果信号量的值是0，将阻塞等待，直到信号量的值大于0。
    bool Semp::wait(short value)
    {
        if (m_semid == -1)
            return false;

        struct sembuf sem_b;
        sem_b.sem_num = 0;    // 信号量编号，0代表第一个信号量。
        sem_b.sem_op = value; // P操作的value必须小于0。
        sem_b.sem_flg = m_sem_flg;
        if (semop(m_semid, &sem_b, 1) == -1)
        {
            LOG_ERROR << "p semop()";
            return false;
        }

        return true;
    }

    // 信号量的V操作（把信号量的值减value）。
    bool Semp::post(short value)
    {
        if (m_semid == -1)
            return false;

        struct sembuf sem_b;
        sem_b.sem_num = 0;    // 信号量编号，0代表第一个信号量。
        sem_b.sem_op = value; // V操作的value必须大于0。
        sem_b.sem_flg = m_sem_flg;
        if (semop(m_semid, &sem_b, 1) == -1)
        {
            LOG_ERROR << "V semop()";
            return false;
        }

        return true;
    }

    // 获取信号量的值，成功返回信号量的值，失败返回-1。
    int Semp::getValue()
    {
        return semctl(m_semid, 0, GETVAL);
    }

    // 销毁信号量。
    bool Semp::destroy()
    {
        if (m_semid == -1)
            return false;

        if (semctl(m_semid, 0, IPC_RMID) == -1)
        {
            perror("destroy semctl()");
            return false;
        }

        return true;
    }

    Semp::~Semp()
    {
    }

}