#ifndef HGL_NETWORK_IP_INCLUDE
#define HGL_NETWORK_IP_INCLUDE

#include<hgl/type/DataType.h>
#include<hgl/type/List.h>
#include<hgl/type/BaseString.h>
#include<hgl/Str.h>

#if HGL_OS == HGL_OS_Windows
    #include<winsock2.h>
    #include<ws2tcpip.h>

    #if SOMAXCONN == 5
    #error Please use <winsock2.h>
    #endif//

    typedef int socklen_t;
    typedef ULONG in_addr_t;

    #define GetLastSocketError() WSAGetLastError()
#else
    #include<errno.h>
    #include<sys/types.h>
    #include<sys/ioctl.h>
    #include<sys/socket.h>
    #include<unistd.h>
    #include<netdb.h>
    #include<arpa/inet.h>
    #include<netinet/in.h>

    #define GetLastSocketError() (errno)

    #if HGL_OS == HGL_OS_Linux
        #include<sys/sendfile.h>

        inline int sendfile(int tfd,int sfd,size_t size)
        {
            return sendfile(tfd,sfd,nullptr,size);
        }
    #endif//HGL_OS == HGL_OS_Linux

    #if HGL_OS == HGL_OS_FreeBSD
        #include<sys/uio.h>

        inline int sendfile(int tfd,int sfd,size_t size)
        {
            return sendfile(tfd,sfd,0,size,nullptr,nullptr,0);
        }
    #endif//HGL_OS == HGL_OS_FreeBSD
#endif//HGL_OS == HGL_OS_Windows

namespace hgl
{
	namespace network
	{
		/**
         * IP地址类
         */
		class IPAddress
		{
        protected:

            int socktype;
            int protocol;

        public:

            IPAddress();

            virtual const int GetFamily()const=0;                                                           ///<返回网络家族
                    const int GetSocketType()const{return socktype;}                                        ///<返回Socket类型
                    const int GetProtocol()const{return protocol;}                                          ///<返回协议类型
            virtual const int GetSockAddrInSize()const=0;                                                   ///<取得SockAddrIn变量长度
            virtual const int GetIPStringMaxSize()const=0;                                                  ///<取得IP字符串最大长度

            /**
             * 设置IP地址
             * @param _name 域名或地址字符串
             * @param _port 端口号
             * @param socktype Socket类型(可以为SOCK_STREAM、SOCK_DGRAM、SOCK_RAW、SOCK_RDM、SOCK_SEQPACKET等值),默认为所有类型。
             * @param protocol 协议类型(可以为IPPROTO_TCP、IPPROTO_UDP、IPPROTO_SCTP),默认为所有类型。
             */
            virtual bool Set(const char *_name,ushort _port,int socktype,int protocol)=0;

                    bool SetTCP (const char *_name,ushort _port){return Set(_name,_port,SOCK_STREAM,    IPPROTO_TCP );}
                    bool SetUDP (const char *_name,ushort _port){return Set(_name,_port,SOCK_DGRAM,     IPPROTO_UDP );}
                    bool SetSCTP(const char *_name,ushort _port){return Set(_name,_port,SOCK_SEQPACKET, IPPROTO_SCTP);}

            /**
             * 设置一个仅有端口号的地址，一般用于服务器监听本机所有地址
             */
            virtual void Set(ushort port)=0;

            /**
             * 绑定当前IP地址到一个socket上
             * @param ThisSocket Socket号
             * @param reuse 是否可以复用这个IP，默认为true
             */
            virtual bool Bind(int ThisSocket,int reuse=1)const=0;

            virtual sockaddr *GetSockAddr()=0;

            /**
             * 取得当前地址的端口号
             */
            virtual const ushort GetPort()const=0;

            /**
             * 转换当前地址到一个可视字符串,字符串所需长度请使用GetIPStringMaxSize()获取
             */
            virtual void ToString(char *)const=0;

            /**
             * 创建一个当前地址的副本
             */
            virtual IPAddress *CreateCopy()const=0;

            /**
             * 创建一个空的IP地址副本
             */
            virtual IPAddress *Create()const=0;
        };//class IPAddress

        /**
         * IPv4地址
         */
        class IPv4Address:public IPAddress
        {
            sockaddr_in addr;

        public:

            IPv4Address(){hgl_zero(addr);}
            IPv4Address(const char *name,ushort port,int _socktype,int _protocol)
            {
                Set(name,port,_socktype,_protocol);
            }
            IPv4Address(const IPv4Address *src)
            {
                hgl_cpy(addr,src->addr);
                socktype=src->socktype;
                protocol=src->protocol;
            }

            const int GetFamily()const{return AF_INET;}
            const int GetSockAddrInSize()const{return sizeof(sockaddr_in);}
            const int GetIPStringMaxSize()const{return INET_ADDRSTRLEN;}

            bool Set(const char *name,ushort port,int _socktype,int _protocol);
            void Set(ushort port);
            bool Bind(int ThisSocket,int reuse=1)const;

            sockaddr *GetSockAddr(){return (sockaddr *)&addr;}

            const ushort GetPort()const;

            void ToString(char *str)const{inet_ntop(AF_INET,&addr,str,INET_ADDRSTRLEN);}

            int GetDomainIPList(List<in_addr> &addr_list,const char *domain,int _socktype,int _protocol);
            int GetLocalIPList(List<in_addr> &addr_list,int _socktype,int _protocol);

            IPAddress *CreateCopy()const
            {
                return(new IPv4Address(this));
            }

            virtual IPAddress *Create()const
            {
                return(new IPv4Address());
            }
        };//class IPv4Address

        /**
         * IPv6地址
         */
        class IPv6Address:public IPAddress
        {
            sockaddr_in6 addr;

        public:

            IPv6Address(){hgl_zero(addr);}
            IPv6Address(const char *name,ushort port,int _socktype,int _protocol)
            {
                Set(name,port,_socktype,_protocol);
            }
            IPv6Address(const IPv6Address *src)
            {
                hgl_cpy(addr,src->addr);
                socktype=src->socktype;
                protocol=src->protocol;
            }

            const int GetFamily()const{return AF_INET6;}
            const int GetSockAddrInSize()const{return sizeof(sockaddr_in6);}
            const int GetIPStringMaxSize()const{return INET6_ADDRSTRLEN;}

            bool Set(const char *name,ushort port,int _socktype,int _protocol);
            void Set(ushort port);
            bool Bind(int ThisSocket,int reuse=1)const;

            sockaddr *GetSockAddr(){return (sockaddr *)&addr;}
            const ushort GetPort()const;

            void ToString(char *str)const{inet_ntop(AF_INET6,&addr,str,INET6_ADDRSTRLEN);}
            int GetDomainIPList(List<in6_addr> &addr_list,const char *domain,int _socktype,int _protocol);
            int GetLocalIPList(List<in6_addr> &addr_list,int _socktype,int _protocol);

            IPAddress *CreateCopy()const
            {
                return(new IPv6Address(this));
            }

            virtual IPAddress *Create()const
            {
                return(new IPv6Address());
            }
        };//class IPv6Address
	}//namespace network
}//namespace hgl
#endif//HGL_NETWORK_IP_TOOL_INCLUDE