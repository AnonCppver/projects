#ifndef _LLEF_NET_TINYTCP_H
#define _LLEF_NET_TINYTCP_H

#include <arpa/inet.h>

namespace leef
{
    namespace net
    {
        namespace TinyTcp
        {
            class TcpClient
            {
            public:
                int m_sockfd;    // 客户端的socket.
                char m_ip[21];   // 服务端的ip地址。
                int m_port;      // 与服务端通信的端口。
                bool m_btimeout; // 调用Read和Write方法时，失败的原因是否是超时：true-未超时，false-已超时。
                int m_buflen;    // 调用Read方法后，接收到的报文的大小，单位：字节。

                TcpClient(); // 构造函数。

                // 向服务端发起连接请求。
                // ip：服务端的ip地址。
                // port：服务端监听的端口。
                // 返回值：true-成功；false-失败。
                bool connectToServer(const char *ip, const int port);

                // 接收服务端发送过来的数据。
                // buffer：接收数据缓冲区的地址，数据的长度存放在m_buflen成员变量中。
                // itimeout：等待数据的超时时间，单位：秒，缺省值是0-无限等待。
                // 返回值：true-成功；false-失败，失败有两种情况：1）等待超时，成员变量m_btimeout的值被设置为true；2）socket连接已不可用。
                bool read(char *buffer, const int itimeout = 0);

                // 向服务端发送数据。
                // buffer：待发送数据缓冲区的地址。
                // ibuflen：待发送数据的大小，单位：字节，缺省值为0，如果发送的是ascii字符串，ibuflen取0，如果是二进制流数据，ibuflen为二进制数据块的大小。
                // 返回值：true-成功；false-失败，如果失败，表示socket连接已不可用。
                bool write(const char *buffer, const int ibuflen = 0);

                // 断开与服务端的连接
                void close();

                bool isConnected();

                ~TcpClient(); // 析构函数自动关闭socket，释放资源。
            };

            // socket通信的服务端类
            class TcpServer
            {
            private:
                int m_socklen;                   // 结构体struct sockaddr_in的大小。
                struct sockaddr_in m_clientaddr; // 客户端的地址信息。
                struct sockaddr_in m_servaddr;   // 服务端的地址信息。
            public:
                int m_listenfd;  // 服务端用于监听的socket。
                int m_connfd;    // 客户端连接上来的socket。
                bool m_btimeout; // 调用Read和Write方法时，失败的原因是否是超时：true-未超时，false-已超时。
                int m_buflen;    // 调用Read方法后，接收到的报文的大小，单位：字节。

                TcpServer(); // 构造函数。

                // 服务端初始化。
                // port：指定服务端用于监听的端口。
                // 返回值：true-成功；false-失败，一般情况下，只要port设置正确，没有被占用，初始化都会成功。
                bool initServer(const unsigned int port);

                // 阻塞等待客户端的连接请求。
                // 返回值：true-有新的客户端已连接上来，false-失败，Accept被中断，如果Accept失败，可以重新Accept。
                bool accept();

                // 获取客户端的ip地址。
                // 返回值：客户端的ip地址，如"192.168.1.100"。
                char *getIP();

                // 接收客户端发送过来的数据。
                // buffer：接收数据缓冲区的地址，数据的长度存放在m_buflen成员变量中。
                // itimeout：等待数据的超时时间，单位：秒，缺省值是0-无限等待。
                // 返回值：true-成功；false-失败，失败有两种情况：1）等待超时，成员变量m_btimeout的值被设置为true；2）socket连接已不可用。
                bool read(char *buffer, const int itimeout);

                // 向客户端发送数据。
                // buffer：待发送数据缓冲区的地址。
                // ibuflen：待发送数据的大小，单位：字节，缺省值为0，如果发送的是ascii字符串，ibuflen取0，如果是二进制流数据，ibuflen为二进制数据块的大小。
                // 返回值：true-成功；false-失败，如果失败，表示socket连接已不可用。
                bool write(const char *buffer, const int ibuflen = 0);

                // 关闭监听的socket，即m_listenfd，常用于多进程服务程序的子进程代码中。
                void closeListen();

                // 关闭客户端的socket，即m_connfd，常用于多进程服务程序的父进程代码中。
                void closeClient();

                ~TcpServer(); // 析构函数自动关闭socket，释放资源。
            };

            // 接收socket的对端发送过来的数据。
            // sockfd：可用的socket连接。
            // buffer：接收数据缓冲区的地址。
            // ibuflen：本次成功接收数据的字节数。
            // itimeout：接收等待超时的时间，单位：秒，缺省值是0-无限等待。
            // 返回值：true-成功；false-失败，失败有两种情况：1）等待超时；2）socket连接已不可用。
            bool tcpRead(const int sockfd, char *buffer, int *ibuflen, const int itimeout = 0);

            // 向socket的对端发送数据。
            // sockfd：可用的socket连接。
            // buffer：待发送数据缓冲区的地址。
            // ibuflen：待发送数据的字节数，如果发送的是ascii字符串，ibuflen取0，如果是二进制流数据，ibuflen为二进制数据块的大小。
            // 返回值：true-成功；false-失败，如果失败，表示socket连接已不可用。
            bool tcpWrite(const int sockfd, const char *buffer, const int ibuflen = 0);

            // 从已经准备好的socket中读取数据。
            // sockfd：已经准备好的socket连接。
            // buffer：接收数据缓冲区的地址。
            // n：本次接收数据的字节数。
            // 返回值：成功接收到n字节的数据后返回true，socket连接不可用返回false。
            bool readn(const int sockfd, char *buffer, const size_t n);

            // 向已经准备好的socket中写入数据。
            // sockfd：已经准备好的socket连接。
            // buffer：待发送数据缓冲区的地址。
            // n：待发送数据的字节数。
            // 返回值：成功发送完n字节的数据后返回true，socket连接不可用返回false。
            bool writen(const int sockfd, const char *buffer, const size_t n);
        }
    }
}

#endif // _LLEF_NET_TINTTCP_H