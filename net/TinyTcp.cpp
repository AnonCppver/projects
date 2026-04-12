#include "TinyTcp.h"

#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/errno.h>

#include <string.h>

namespace leef::net::TinyTcp
{

TcpClient::TcpClient()
{
    m_sockfd = -1;
    memset(m_ip, 0, sizeof(m_ip));
    m_port = 0;
    m_btimeout = false;
}

bool TcpClient::connectToServer(const char *ip, const int port)
{
    if (m_sockfd != -1)
    {
        ::close(m_sockfd);
        m_sockfd = -1;
    }

    strcpy(m_ip, ip);
    m_port = port;

    struct hostent *h;
    struct sockaddr_in servaddr;

    if ((m_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return false;

    if (!(h = gethostbyname(m_ip)))
    {
        ::close(m_sockfd);
        m_sockfd = -1;
        return false;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(m_port); // 指定服务端的通讯端口
    memcpy(&servaddr.sin_addr, h->h_addr, h->h_length);

    if (connect(m_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        ::close(m_sockfd);
        m_sockfd = -1;
        return false;
    }

    return true;
}

bool TcpClient::read(char *buffer, const int itimeout)
{
    if (m_sockfd == -1)
        return false;

    if (itimeout > 0)
    {
        fd_set tmpfd;

        FD_ZERO(&tmpfd);
        FD_SET(m_sockfd, &tmpfd);

        struct timeval timeout;
        timeout.tv_sec = itimeout;
        timeout.tv_usec = 0;

        m_btimeout = false;

        int i;
        if ((i = select(m_sockfd + 1, &tmpfd, 0, 0, &timeout)) <= 0)
        {
            if (i == 0)
                m_btimeout = true;
            return false;
        }
    }

    m_buflen = 0;
    return (tcpRead(m_sockfd, buffer, &m_buflen));
}

bool TcpClient::write(const char *buffer, const int ibuflen)
{
    if (m_sockfd == -1)
        return false;

    fd_set tmpfd;

    FD_ZERO(&tmpfd);
    FD_SET(m_sockfd, &tmpfd);

    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    m_btimeout = false;

    int i;
    if ((i = select(m_sockfd + 1, 0, &tmpfd, 0, &timeout)) <= 0)
    {
        if (i == 0)
            m_btimeout = true;
        return false;
    }

    int ilen = ibuflen;

    if (ibuflen == 0)
        ilen = strlen(buffer);

    return (tcpWrite(m_sockfd, buffer, ilen));
}

void TcpClient::close()
{
    if (m_sockfd > 0)
        ::close(m_sockfd);

    m_sockfd = -1;
    memset(m_ip, 0, sizeof(m_ip));
    m_port = 0;
    m_btimeout = false;
}

bool TcpClient::isConnected()
{
    char buf;
    int ret = recv(m_sockfd, &buf, 1, MSG_PEEK | MSG_DONTWAIT);

    if (ret > 0)
        return true;

    if (ret == 0)
        return false; // 对端关闭

    if (errno == EAGAIN || errno == EWOULDBLOCK)
        return true; // 没数据但连接还在

    return false;
}

TcpClient::~TcpClient()
{
    close();
}

TcpServer::TcpServer()
{
    m_listenfd = -1;
    m_connfd = -1;
    m_socklen = 0;
    m_btimeout = false;
}

bool TcpServer::initServer(const unsigned int port)
{
    if (m_listenfd > 0)
    {
        ::close(m_listenfd);
        m_listenfd = -1;
    }

    m_listenfd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    unsigned int len = sizeof(opt);
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, len);
    setsockopt(m_listenfd, SOL_SOCKET, SO_KEEPALIVE, &opt, len);

    memset(&m_servaddr, 0, sizeof(m_servaddr));
    m_servaddr.sin_family = AF_INET;
    m_servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    m_servaddr.sin_port = htons(port);
    if (bind(m_listenfd, (struct sockaddr *)&m_servaddr, sizeof(m_servaddr)) != 0)
    {
        closeListen();
        return false;
    }

    if (listen(m_listenfd, 5) != 0)
    {
        closeListen();
        return false;
    }

    m_socklen = sizeof(struct sockaddr_in);

    return true;
}

bool TcpServer::accept()
{
    if (m_listenfd == -1)
        return false;

    if ((m_connfd = ::accept(m_listenfd, (struct sockaddr *)&m_clientaddr, (socklen_t *)&m_socklen)) < 0)
        return false;

    return true;
}

char *TcpServer::getIP()
{
    return (inet_ntoa(m_clientaddr.sin_addr));
}

bool TcpServer::read(char *buffer, const int itimeout)
{
    if (m_connfd == -1)
        return false;

    if (itimeout > 0)
    {
        fd_set tmpfd;

        FD_ZERO(&tmpfd);
        FD_SET(m_connfd, &tmpfd);

        struct timeval timeout;
        timeout.tv_sec = itimeout;
        timeout.tv_usec = 0;

        m_btimeout = false;

        int i;
        if ((i = select(m_connfd + 1, &tmpfd, 0, 0, &timeout)) <= 0)
        {
            if (i == 0)
                m_btimeout = true;
            return false;
        }
    }

    m_buflen = 0;
    return (tcpRead(m_connfd, buffer, &m_buflen));
}

bool TcpServer::write(const char *buffer, const int ibuflen)
{
    if (m_connfd == -1)
        return false;

    fd_set tmpfd;

    FD_ZERO(&tmpfd);
    FD_SET(m_connfd, &tmpfd);

    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    m_btimeout = false;

    int i;
    if ((i = select(m_connfd + 1, 0, &tmpfd, 0, &timeout)) <= 0)
    {
        if (i == 0)
            m_btimeout = true;
        return false;
    }

    int ilen = ibuflen;
    if (ilen == 0)
        ilen=strlen(buffer);

    return (tcpWrite(m_connfd, buffer, ilen));
}

void TcpServer::closeListen()
{
    if (m_listenfd > 0)
    {
        ::close(m_listenfd);
        m_listenfd = -1;
    }
}

void TcpServer::closeClient()
{
    if (m_connfd > 0)
    {
        ::close(m_connfd);
        m_connfd = -1;
    }
}

TcpServer::~TcpServer()
{
    closeListen();
    closeClient();
}

bool tcpRead(const int sockfd, char *buffer, int *ibuflen, const int itimeout)
{
    if (sockfd == -1)
        return false;

    if (itimeout > 0)
    {
        fd_set tmpfd;

        FD_ZERO(&tmpfd);
        FD_SET(sockfd, &tmpfd);

        struct timeval timeout;
        timeout.tv_sec = itimeout;
        timeout.tv_usec = 0;

        int i;
        if ((i = select(sockfd + 1, &tmpfd, 0, 0, &timeout)) <= 0)
            return false;
    }

    (*ibuflen) = 0;

    if (readn(sockfd, (char *)ibuflen, 4) == false)
        return false;

    if (readn(sockfd, buffer, (*ibuflen)) == false)
        return false;

    return true;
}

bool tcpWrite(const int sockfd, const char *buffer, const int ibuflen)
{
    if (sockfd == -1)
        return false;

    fd_set tmpfd;

    FD_ZERO(&tmpfd);
    FD_SET(sockfd, &tmpfd);

    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    if (select(sockfd + 1, 0, &tmpfd, 0, &timeout) <= 0)
        return false;

    int ilen = 0;

    // 如果长度为0，就采用字符串的长度
    if (ibuflen == 0)
        ilen = strlen(buffer);
    else
        ilen = ibuflen;

    char strTBuffer[ilen + 4];
    memset(strTBuffer, 0, sizeof(strTBuffer));
    memcpy(strTBuffer, &ilen, 4);
    memcpy(strTBuffer + 4, buffer, ilen);

    if (writen(sockfd, strTBuffer, ilen + 4) == false)
        return false;

    return true;
}

bool readn(const int sockfd, char *buffer, const size_t n)
{
    int nLeft, nread, idx;

    nLeft = n;
    idx = 0;

    while (nLeft > 0)
    {
        if ((nread = recv(sockfd, buffer + idx, nLeft, 0)) <= 0)
            return false;

        idx += nread;
        nLeft -= nread;
    }

    return true;
}

bool writen(const int sockfd, const char *buffer, const size_t n)
{
    int nLeft, idx, nwritten;
    nLeft = n;
    idx = 0;
    while (nLeft > 0)
    {
        if ((nwritten = send(sockfd, buffer + idx, nLeft, 0)) <= 0)
            return false;

        nLeft -= nwritten;
        idx += nwritten;
    }

    return true;
}
}