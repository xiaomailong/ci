/*********************************************************************
Copyright (C), 2011,  Co.Hengjun, Ltd.

作者        : ZHANGB
版本        : 1.0
创建日期    : 2015年9月24日 10:42:29
用途        : 集中控制管理
历史修改记录:
**********************************************************************/
#include "centralized_manage.h"
#include "syn_data/centralized_syn.h"

#include "util/config.h"
#include "util/utility.h"
#include "util/app_config.h"
#include "util/utility.h"
#include "util/log.h"
#include "cpu_manage.h"
#include "interlocking/global_data.h"

#ifdef WIN32
#   pragma comment(lib,"WS2_32.lib")

static SOCKET centralized_socket_from_station1; /*在windows下创建socket使用网络*/
static SOCKET centralized_socket_from_station2;/*在windows下创建socket使用网络*/
static SOCKET centralized_socket_send; /*在windows下创建socket使用网络*/
#else
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

static int32_t centralized_sockto = 0;            /*集中控制通过PC104网口发送数据的文件句柄*/
static int32_t centralized_sockfrom_station1 = 0;            /*集中控制通过PC104网口接收数据的文件句柄*/
static int32_t centralized_sockfrom_station2= 0;            /*集中控制通过PC104网口接收数据的文件句柄*/

#endif /* WIN32*/

const int32_t STATION1 = 1;
const int32_t STATION2 = 2;

static struct sockaddr_in centralized_socket_master_addrto1;            /*集中控制发送的站1主系地址*/
static struct sockaddr_in centralized_socket_master_addrto2;            /*集中控制发送的站2主系地址*/
static struct sockaddr_in centralized_socket_standby_addrto1;            /*集中控制发送的站1备系地址*/
static struct sockaddr_in centralized_socket_standby_addrto2;            /*集中控制发送的站2备系地址*/
static struct sockaddr_in centralized_socket_addrfrom;            /*集中控制通过PC104网口接收数据的地址*/

static CI_BOOL IsDoubleStation = CI_FALSE; /*是否与两个站交互集中控制数据*/

static const char* centralized_master_ip_send1 = NULL;            /*集中控制发送的站1主系ip地址*/
static const char* centralized_master_ip_recive1 = NULL;            /*集中控制接收的站1主系ip地址*/
static const char* centralized_master_ip_send2 = NULL;            /*集中控制发送的站2主系ip地址*/
static const char* centralized_master_ip_recive2 = NULL;            /*集中控制接收的站2主系ip地址*/

static const char* centralized_standby_ip_send1 = NULL;            /*集中控制发送的站1备系ip地址*/
static const char* centralized_standby_ip_recive1 = NULL;            /*集中控制接收站1备系的ip地址*/
static const char* centralized_standby_ip_send2 = NULL;            /*集中控制发送的站2备系ip地址*/
static const char* centralized_standby_ip_recive2 = NULL;            /*集中控制接收站2备系的ip地址*/

static const char* centralized_local_ip = NULL;            /*集中控制绑定的本地ip地址*/

static uint16_t centralized_port_station1 = 0;           /*集中控制站1数据的端口地址*/
static uint16_t centralized_port_station2 = 0;           /*集中控制站2数据的端口地址*/

static CentralizedFrame self1_centralized_frame, self2_centralized_frame, peer1_centralized_frame, peer2_centralized_frame;  /*集中控制数据*/
#define MAX_CENTRALIZED_LENGTH 1024 /*集中控制数据网络最大通信量*/


/*
功能描述    : 跨场作业数据装配
返回值      : 成功为0，失败为-1
参数        :
日期        : 2015年9月14日
*/
static int32_t centralized_assemble_data(void* dist, CentralizedFrame *frame)
{
    int32_t len = 0;
#define CENTRALIZED_SEND_MEMCPY(data)                       \
    memcpy((char*)dist + len,&data,sizeof(data));   \
    len += sizeof(data);

    CENTRALIZED_SEND_MEMCPY(*frame)
#undef CENTRALIZED_SEND_MEMCPY
        return len;
}

/*
功能描述    : 清除udp缓存
日期        : 2015年9月14日
*/
static void clear_udp_buf(void)
{
    static char dummy_buf[65535];
#ifdef LINUX_ENVRIONMENT
    while (0 < recvfrom(centralized_sockfrom_station1, dummy_buf, 65535, 0, NULL, NULL));
    while (0 < recvfrom(centralized_sockfrom_station2, dummy_buf, 65535, 0, NULL, NULL));
#else
    while (0 < recvfrom(centralized_socket_from_station1, dummy_buf, 65535, 0, NULL, NULL));
    while (0 < recvfrom(centralized_socket_from_station2, dummy_buf, 65535, 0, NULL, NULL));
#endif
}

/*
功能描述    : 数据检查
日期        : 2015年9月14日
*/
static int32_t centralized_check_data(CentralizedFrame dateframe, int32_t stationnum)
{
    if (CICentralizedSyn_Verify(&dateframe) == CI_FALSE)
    {
        CILog_Msg("cross station data%d crc err", stationnum);
        return -1;
    }
    return 0;
}

/*
功能描述    : 写跨场作业数据
返回值      : 成功为0，失败为-1
参数        :frame--数据帧      ， addr_master--主系地址，        addr_standy--备系地址
日期        : 2015年9月14日
*/
static int32_t centralized_write_data(const CentralizedFrame frame ,const struct sockaddr_in addr_master, const struct sockaddr_in addr_standy)
{
    int32_t ret;
    static uint8_t send_buff[MAX_CENTRALIZED_LENGTH] = { 0 };
    int buflen = centralized_assemble_data(send_buff, &frame);
#ifdef LINUX_ENVRIONMENT
    ret = sendto(centralized_sockto, send_buff, buflen, 0, (struct sockaddr *)&addr_master, sizeof(addr_master));
#else
    ret = sendto(centralized_socket_send, (const char*)send_buff, buflen, 0, (struct sockaddr *)&addr_master, sizeof(addr_master));
#endif
    if (0 > ret)
    {
        CILog_Msg("cross station data %d send err", frame.frame_type);
    }
#ifdef LINUX_ENVRIONMENT
    ret = sendto(centralized_sockto, send_buff, buflen, 0, (struct sockaddr *)&addr_standy, sizeof(addr_standy));
    if (0 > ret)
    {
        CILog_Msg("cross station data %d send standby err", frame.frame_type);
    }
#endif
    if (ret > 0)
    {
        CILog_Msg("cross station data %d send:%d ", frame.frame_type, frame.frame_sn);
    }
    return ret;
}

/*
功能描述    : 读跨场作业数据
返回值      : 成功为0，失败为-1
参数        :
日期        : 2015年9月14日
*/
static int32_t centralized_read_data(CentralizedFrame *frame, int stationnum)
{
    int32_t read_num = -1;
#ifdef LINUX_ENVRIONMENT
    static socklen_t addrlen = sizeof(centralized_socket_addrfrom);
#else
    int32_t fromelen = sizeof(centralized_socket_addrfrom);
#endif
    memset(frame, 0, sizeof(*frame));
    memset(&centralized_socket_addrfrom, 0, sizeof(centralized_socket_addrfrom));
#ifdef LINUX_ENVRIONMENT
    if (stationnum == STATION2)
    {
        read_num = recvfrom(centralized_sockfrom_station2, frame, sizeof(*frame), 0, (struct sockaddr *)&centralized_socket_addrfrom, &addrlen);
    }
    else if (stationnum == STATION1)
    {
        read_num = recvfrom(centralized_sockfrom_station1, frame, sizeof(*frame), 0, (struct sockaddr *)&centralized_socket_addrfrom, &addrlen);
    }
#else
    if (stationnum == STATION2)
    {
        read_num = recvfrom(centralized_socket_from_station2, (char*)frame, sizeof(*frame), 0, (struct sockaddr *)&centralized_socket_addrfrom, &fromelen);
    }
    else if (stationnum == STATION1)
    {
        read_num = recvfrom(centralized_socket_from_station1, (char*)frame, sizeof(*frame), 0, (struct sockaddr *)&centralized_socket_addrfrom, &fromelen);
    }
#endif
    if (read_num < 0)
    {
#ifdef LINUX_ENVRIONMENT
        CILog_Msg("%s  cross station%d data recive err", CISeries_GetStateName(CISeries_GetLocalState()),stationnum);
#else
        CILog_Msg("cross station%d data recive err" , stationnum);
#endif
    }
    return read_num;
}

/*
功能描述    : 发送跨场作业数据
返回值      : 成功为0，失败为-1
参数        :
日期        : 2015年9月14日
*/
int32_t CICentralized_SendData()
{
    int32_t ret = 0;
    //只有主系主CPU发送
    if (CICpu_GetLocalState() != CPU_STATE_MASTER || CISeries_GetLocalState() != SERIES_MASTER)
    {
        return 0;
    }
    //先装配，后发送
    CICentralizedSyn1_Assemble(&self1_centralized_frame);
    ret = centralized_write_data(self1_centralized_frame, centralized_socket_master_addrto1, centralized_socket_standby_addrto1);
    if (0 > ret)
    {
        if (CI_FALSE == IsDoubleStation)
        {
            return -1;
        }
    }
    if (CI_TRUE == IsDoubleStation)
    {
        //先装配，后发送
        CICentralizedSyn2_Assemble(&self2_centralized_frame);
        ret = centralized_write_data(self2_centralized_frame, centralized_socket_master_addrto2, centralized_socket_standby_addrto2);
        if (0 > ret)
        {
            return -1;
        }
    }
    return 0;
}

/*
功能描述    : 接收跨场作业数据
返回值      : 成功为0，失败为-1
参数        :
日期        : 2015年9月14日
*/
int32_t CICentralized_RecvData()
{
    int32_t ret;
    //只有主CPU接收
    if (CICpu_GetLocalState() != CPU_STATE_MASTER)
    {
        return 0;
    }
    memset(&peer1_centralized_frame, 0, sizeof(peer1_centralized_frame));
    ret = centralized_read_data(&peer1_centralized_frame,STATION1);  //读取
    //CILog_Msg("test1");
    if (0 > ret)   //未收到数据
    {
        if (CI_FALSE == IsDoubleStation)
        {
            clear_udp_buf();
            return -1;
        }
    }
    else   //收到数据
    {
        if ((centralized_socket_addrfrom.sin_addr.s_addr == inet_addr(centralized_master_ip_recive1))
            || (centralized_socket_addrfrom.sin_addr.s_addr == inet_addr(centralized_standby_ip_recive1)))
        {
            if (peer1_centralized_frame.frame_head_tag == 0xaa55)
            {
                ret = centralized_check_data(peer1_centralized_frame, STATION1);
                if (0 == ret)
                {
                    CILog_Msg("cross station1 data recive: %d", peer1_centralized_frame.frame_sn);
                    //解析
                    CICentralizedSyn1_Analysise(&peer1_centralized_frame);
                }
            }
            else
            {
                CILog_Msg("cross station data1 recived but frame_head err");
            }
        }
        else
        {
            CILog_Msg("cross station1 dirty data");
        }
    }

    if (CI_TRUE == IsDoubleStation)
    {
        memset(&peer2_centralized_frame, 0, sizeof(peer2_centralized_frame));
        ret = centralized_read_data(&peer2_centralized_frame,STATION2);  //读取
        //CILog_Msg("test2");
        if (0 > ret)
        {
            clear_udp_buf();
            return -1;
        }
        else
        {
            if ((centralized_socket_addrfrom.sin_addr.s_addr == inet_addr(centralized_master_ip_recive2))
                || (centralized_socket_addrfrom.sin_addr.s_addr == inet_addr(centralized_standby_ip_recive2)))
            {
                if (peer2_centralized_frame.frame_head_tag == 0xaa55)
                {
                    ret = centralized_check_data(peer2_centralized_frame, STATION2);
                    if (0 == ret)
                    {
                        CILog_Msg("cross station data2 recive: %d", peer2_centralized_frame.frame_sn);
                        //解析
                        CICentralizedSyn2_Analysise(&peer2_centralized_frame);
                    }
                }
                else
                {
                    CILog_Msg("cross station data2 recived but frame_head err");
                }
            }
            else
            {
                CILog_Msg("cross station2 dirty data");
            }
        }

    }
    clear_udp_buf();
    return 0;
}

/*
功能描述    : 获取集中控制各端口
返回值      : 成功为0，失败为-1
参数        :
日期        : 2015年9月14日
*/
static int32_t centralized_get_port(const char* config_name, uint16_t* port)
{
    const char* centralized_port_str = NULL;
    *port = 0;
    /*try get hmi port from configuration file*/
    centralized_port_str = CIConfig_GetValue(config_name);
    if (NULL == centralized_port_str)
    {
        CILog_Msg("配置文件当中找不到%s项", config_name);
        return -1;
    }
    /*在未强制转换前验证一次控显机端口，如果其大于65535则报错*/
    if (CI_FALSE == CI_ValidatePort(atoi(centralized_port_str)))
    {
        CILog_Msg("在配置文件当中%s值不符合规范", config_name);
        return -1;
    }
    *port = (uint16_t)atoi(centralized_port_str);
    return 0;
}

/*
功能描述    : 获取集中控制各ip
返回值      : 成功为0，失败为-1
参数        :
日期        : 2015年9月14日
*/
static const char* centralized_get_ip(const char* config_name)
{
    const char* centralized_ip_str = NULL;
    /*try get ip address from configuration file*/
    centralized_ip_str = CIConfig_GetValue(config_name);
    if (NULL == centralized_ip_str)
    {
        CILog_Msg("配置文件当中找不到%s项", config_name);
        return NULL;
    }
    if (CI_FALSE == CI_ValidateIpAddress(centralized_ip_str))
    {
        CILog_Msg("配置文件当中%s项值不符合ip规范", config_name);
        return NULL;
    }
    return centralized_ip_str;
}

/*
功能描述    : 初始化集中控制各ip及端口
返回值      : 成功为0，失败为-1
参数        :
日期        : 2015年9月14日
*/
static int32_t centralized_ip_port_init(void){
    int32_t ret = 0;

    centralized_master_ip_send1 = centralized_get_ip("Centralized_ToStation1_MasterIp");
    if (centralized_master_ip_send1 == NULL)
    {
        return -1;
    }
    centralized_master_ip_recive1 = centralized_get_ip("Centralized_FromStation1_MasterIp");
    if (centralized_master_ip_recive1 == NULL)
    {
        return -1;
    }
    centralized_standby_ip_send1 = centralized_get_ip("Centralized_ToStation1_StandbyIp");
    if (centralized_standby_ip_send1 == NULL)
    {
        return -1;
    }
    centralized_standby_ip_recive1 = centralized_get_ip("Centralized_FromStation1_standbyIp");
    if (centralized_standby_ip_recive1 == NULL)
    {
        return -1;
    }

    centralized_master_ip_send2 = centralized_get_ip("Centralized_ToStation2_MasterIp");
    if (centralized_master_ip_send2 == NULL)
    {
        return -1;
    }
    centralized_master_ip_recive2 = centralized_get_ip("Centralized_FromStation2_MasterIp");
    if (centralized_master_ip_recive2 == NULL)
    {
        return -1;
    }
    centralized_standby_ip_send2 = centralized_get_ip("Centralized_ToStation2_StandbyIp");
    if (centralized_standby_ip_send2 == NULL)
    {
        return -1;
    }
    centralized_standby_ip_recive2 = centralized_get_ip("Centralized_FromStation2_standbyIp");
    if (centralized_standby_ip_recive2 == NULL)
    {
        return -1;
    }

    centralized_local_ip = centralized_get_ip("Centralized_Local_Ip");
    if (centralized_local_ip == NULL)
    {
        return -1;
    }

    ret = centralized_get_port("CentralizedPortStation1", &centralized_port_station1);
    if (0 > ret)
    {
        return -1;
    }
    ret = centralized_get_port("CentralizedPortStation2", &centralized_port_station2);
    if (0 > ret)
    {
        return -1;
    }

    return 0;
}

/*
功能描述    : 初始化集中控制是否与多站交互
返回值      : 成功为0，失败为-1
参数        :
日期        : 2015年9月14日
*/
static int32_t doubleStation_init(){
    const char* double_station = NULL;
    double_station = CIConfig_GetValue("IsDoubleStation");
    if (NULL == double_station)
    {
        CILog_Msg("配置文件当中找不到IsDoubleStation项");
        return -1;
    }

    if (strcmp(double_station, "YES") == 0)
    {
        IsDoubleStation = CI_TRUE;
    }
    else if (strcmp(double_station, "NO") == 0)
    {
        IsDoubleStation = CI_FALSE;
    }
    else
    {
        CILog_Msg("IsDoubleStation项配置错误");
        return -1;
    }

    return 0;
}


/*
功能描述    : 从配置文件判断是否跨场作业
返回值      : 成功为0，失败为-1
参数        :是否跨场作业标志
作者        : 张博
日期        : 2015年9月14日
*/
int32_t CICentralized_IsDo(CI_BOOL *isdo)
{
    const char* cross_station = NULL;
    cross_station = CIConfig_GetValue("IsCrossStation");
    if (NULL == cross_station)
    {
        CILog_Msg("配置文件当中找不到IsCrossStation项");
        return -1;
    }

    if (strcmp(cross_station, "YES") == 0)
    {
        *isdo = CI_TRUE;
    }
    else if (strcmp(cross_station, "NO") == 0)
    {
        *isdo = CI_FALSE;
    }
    else
    {
        CILog_Msg("IsCrossStation项配置错误");
        return -1;
    }
    return 0;
}

#ifdef WIN32
/*
功能描述    : 在windows下使用socket直接与控显机进行通信
返回值      : 成功为0，失败为-1
参数        :
日期        : 2015年9月21日
*/
static int32_t centralized_socket_init_windows(void)
{

    int32_t ret = 0;
    WSADATA            wsd;            //WSADATA变量
    static struct sockaddr_in centralized_socket_addrlocal1;        //服务器地址
    static struct sockaddr_in centralized_socket_addrlocal2;        //服务器地址
    unsigned long ul = 1;                       /*1 block,0 nonblock*/

    //初始化套结字动态库
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        CILog_Msg("初始化套接字动态库失败!\n");
        return -1;
    }

    //创建套接字
    centralized_socket_send = socket(AF_INET, SOCK_DGRAM, 0);
    if (centralized_socket_send == INVALID_SOCKET)
    {
        CILog_Msg("创建套接字失败，失败原因; %d\n", WSAGetLastError());
        WSACleanup();//释放套接字资源
        return -1;
    }

    //创建套接字
    centralized_socket_from_station1 = socket(AF_INET, SOCK_DGRAM, 0);
    if (centralized_socket_from_station1 == INVALID_SOCKET)
    {
        CILog_Msg("创建套接字失败，失败原因; %d\n", WSAGetLastError());
        WSACleanup();//释放套接字资源
        return -1;
    }

    //创建套接字
    centralized_socket_from_station2 = socket(AF_INET, SOCK_DGRAM, 0);
    if (centralized_socket_from_station2 == INVALID_SOCKET)
    {
        CILog_Msg("创建套接字失败，失败原因; %d\n", WSAGetLastError());
        WSACleanup();//释放套接字资源
        return -1;
    }

    //服务器地址
    centralized_socket_addrlocal1.sin_family = AF_INET;
    centralized_socket_addrlocal1.sin_port = htons((short)centralized_port_station1);            //端口
    centralized_socket_addrlocal1.sin_addr.s_addr = inet_addr(centralized_local_ip);    //IP

    centralized_socket_addrlocal2.sin_family = AF_INET;
    centralized_socket_addrlocal2.sin_port = htons((short)centralized_port_station2);            //端口
    centralized_socket_addrlocal2.sin_addr.s_addr = inet_addr(centralized_local_ip);    //IP


    //发送地址1
    centralized_socket_master_addrto1.sin_family = AF_INET;
    centralized_socket_master_addrto1.sin_port = htons((short)centralized_port_station1);            //端口
    centralized_socket_master_addrto1.sin_addr.s_addr = inet_addr(centralized_master_ip_send1);    //IP
    //发送地址2
    centralized_socket_master_addrto2.sin_family = AF_INET;
    centralized_socket_master_addrto2.sin_port = htons((short)centralized_port_station2);            //端口
    centralized_socket_master_addrto2.sin_addr.s_addr = inet_addr(centralized_master_ip_send2);    //IP

    //绑定
    if (bind(centralized_socket_from_station1, (SOCKADDR *)&centralized_socket_addrlocal1, sizeof(centralized_socket_addrlocal1)) == SOCKET_ERROR)
    {
        CILog_Msg("绑定失败，失败原因: %d\n", WSAGetLastError());
        closesocket(centralized_socket_from_station1);    //关闭套接字
        WSACleanup();    //释放套接字资源
        return -1;
    }

    //绑定
    if (bind(centralized_socket_from_station2, (SOCKADDR *)&centralized_socket_addrlocal2, sizeof(centralized_socket_addrlocal2)) == SOCKET_ERROR)
    {
        CILog_Msg("绑定失败，失败原因: %d\n", WSAGetLastError());
        closesocket(centralized_socket_from_station2);    //关闭套接字
        WSACleanup();    //释放套接字资源
        return -1;
    }

    /*设置成非阻塞模式*/
    ret = ioctlsocket(centralized_socket_from_station1, FIONBIO, (unsigned long *)&ul);
    if (SOCKET_ERROR == ret)
    {
        CILog_Msg("设置非阻塞失败");
        closesocket(centralized_socket_from_station1);
        return -1;
    }

    /*设置成非阻塞模式*/
    ret = ioctlsocket(centralized_socket_from_station2, FIONBIO, (unsigned long *)&ul);
    if (SOCKET_ERROR == ret)
    {
        CILog_Msg("设置非阻塞失败");
        closesocket(centralized_socket_from_station2);
        return -1;
    }


    /*设置成非阻塞模式*/
    ret = ioctlsocket(centralized_socket_send, FIONBIO, (unsigned long *)&ul);
    if (SOCKET_ERROR == ret)
    {
        CILog_Msg("设置非阻塞失败");
        closesocket(centralized_socket_send);
        return -1;
    }

    return 0;
}

#else
/*
功能描述    : 集中控制网络套接字初始化
返回值      : 成功为0，失败为-1
参数        :
日期        : 2015年9月14日
*/
static int32_t centralized_socket_init(void)
{
    int flags;

    centralized_sockto = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == centralized_sockto)
    {
        CILog_Msg("socket centralized_sockto failed");

        return -1;
    }
    /*远程ip1设置*/
    bzero(&centralized_socket_master_addrto1, sizeof(struct sockaddr_in));
    centralized_socket_master_addrto1.sin_family = AF_INET;
    centralized_socket_master_addrto1.sin_addr.s_addr = inet_addr(centralized_master_ip_send1);
    centralized_socket_master_addrto1.sin_port = htons(centralized_port_station1);
    /*远程ip2设置*/
    bzero(&centralized_socket_master_addrto2, sizeof(struct sockaddr_in));
    centralized_socket_master_addrto2.sin_family = AF_INET;
    centralized_socket_master_addrto2.sin_addr.s_addr = inet_addr(centralized_master_ip_send2);
    centralized_socket_master_addrto2.sin_port = htons(centralized_port_station2);
    /*远程ip3设置*/
    bzero(&centralized_socket_standby_addrto1, sizeof(struct sockaddr_in));
    centralized_socket_standby_addrto1.sin_family = AF_INET;
    centralized_socket_standby_addrto1.sin_addr.s_addr = inet_addr(centralized_standby_ip_send1);
    centralized_socket_standby_addrto1.sin_port = htons(centralized_port_station1);
    /*远程ip4设置*/
    bzero(&centralized_socket_standby_addrto2, sizeof(struct sockaddr_in));
    centralized_socket_standby_addrto2.sin_family = AF_INET;
    centralized_socket_standby_addrto2.sin_addr.s_addr = inet_addr(centralized_standby_ip_send2);
    centralized_socket_standby_addrto2.sin_port = htons(centralized_port_station2);
    
    centralized_sockfrom_station1 = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == centralized_sockfrom_station1)
    {
        CILog_Msg("socket centralized_sockfrom1 failed");
        return -1;
    }
    /*本地监听ip及端口*/
    static struct sockaddr_in centralized_socket_addrlocal_station1;
    bzero(&centralized_socket_addrlocal_station1, sizeof(struct sockaddr_in));
    centralized_socket_addrlocal_station1.sin_family = AF_INET;
    centralized_socket_addrlocal_station1.sin_addr.s_addr = inet_addr(centralized_local_ip);
    centralized_socket_addrlocal_station1.sin_port = htons(centralized_port_station1);
    /*绑定*/
    if (bind(centralized_sockfrom_station1, (struct sockaddr *)&(centralized_socket_addrlocal_station1), sizeof(struct sockaddr_in)) == -1)
    {
        CILog_Msg("socket1 bind failed");
        return -1;
    }
    flags = fcntl(centralized_sockfrom_station1, F_GETFL, 0);
    fcntl(centralized_sockfrom_station1, F_SETFL, flags | O_NONBLOCK);

    centralized_sockfrom_station2 = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == centralized_sockfrom_station2)
    {
        CILog_Msg("socket centralized_sockfrom2 failed");
        return -1;
    }
    /*本地监听ip及端口*/
    static struct sockaddr_in centralized_socket_addrlocal_station2;
    bzero(&centralized_socket_addrlocal_station2, sizeof(struct sockaddr_in));
    centralized_socket_addrlocal_station2.sin_family = AF_INET;
    centralized_socket_addrlocal_station2.sin_addr.s_addr = inet_addr(centralized_local_ip);
    centralized_socket_addrlocal_station2.sin_port = htons(centralized_port_station2);
    /*绑定*/
    if (bind(centralized_sockfrom_station2, (struct sockaddr *)&(centralized_socket_addrlocal_station2), sizeof(struct sockaddr_in)) == -1)
    {
        CILog_Msg("socket2 bind failed");
        return -1;
    }
    flags = fcntl(centralized_sockfrom_station2, F_GETFL, 0);
    fcntl(centralized_sockfrom_station2, F_SETFL, flags | O_NONBLOCK);

    return 0;
}


#endif /* WIN32*/

/*
功能描述    : 初始化集中控制相关数据
返回值      : 成功为0，失败为-1
参数        :
日期        : 2015年9月14日
*/
int32_t CICentralized_Init(void){
    int ret = 0;

    ret = doubleStation_init();
    if (0 > ret)
    {
        return -1;
    }
    ret = centralized_ip_port_init();
    if (0 > ret)
    {
        return -1;
    }

#ifdef WIN32
    ret = centralized_socket_init_windows();
    if (0 > ret)
    {
        return -1;
    }
#else
    ret = centralized_socket_init();
    if (0 > ret)
    {
        return -1;
    }
#endif

    return 0;
}

