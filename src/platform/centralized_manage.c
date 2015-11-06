/*********************************************************************
Copyright (C), 2011,  Co.Hengjun, Ltd.

����        : ZHANGB
�汾        : 1.0
��������    : 2015��9��24�� 10:42:29
��;        : ���п��ƹ���
��ʷ�޸ļ�¼:
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

static SOCKET centralized_socket_from_station1; /*��windows�´���socketʹ������*/
static SOCKET centralized_socket_from_station2;/*��windows�´���socketʹ������*/
static SOCKET centralized_socket_send; /*��windows�´���socketʹ������*/
#else
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

static int32_t centralized_sockto = 0;            /*���п���ͨ��PC104���ڷ������ݵ��ļ����*/
static int32_t centralized_sockfrom_station1 = 0;            /*���п���ͨ��PC104���ڽ������ݵ��ļ����*/
static int32_t centralized_sockfrom_station2= 0;            /*���п���ͨ��PC104���ڽ������ݵ��ļ����*/

#endif /* WIN32*/

const int32_t STATION1 = 1;
const int32_t STATION2 = 2;

static struct sockaddr_in centralized_socket_master_addrto1;            /*���п��Ʒ��͵�վ1��ϵ��ַ*/
static struct sockaddr_in centralized_socket_master_addrto2;            /*���п��Ʒ��͵�վ2��ϵ��ַ*/
static struct sockaddr_in centralized_socket_standby_addrto1;            /*���п��Ʒ��͵�վ1��ϵ��ַ*/
static struct sockaddr_in centralized_socket_standby_addrto2;            /*���п��Ʒ��͵�վ2��ϵ��ַ*/
static struct sockaddr_in centralized_socket_addrfrom;            /*���п���ͨ��PC104���ڽ������ݵĵ�ַ*/

static CI_BOOL IsDoubleStation = CI_FALSE; /*�Ƿ�������վ�������п�������*/

static const char* centralized_master_ip_send1 = NULL;            /*���п��Ʒ��͵�վ1��ϵip��ַ*/
static const char* centralized_master_ip_recive1 = NULL;            /*���п��ƽ��յ�վ1��ϵip��ַ*/
static const char* centralized_master_ip_send2 = NULL;            /*���п��Ʒ��͵�վ2��ϵip��ַ*/
static const char* centralized_master_ip_recive2 = NULL;            /*���п��ƽ��յ�վ2��ϵip��ַ*/

static const char* centralized_standby_ip_send1 = NULL;            /*���п��Ʒ��͵�վ1��ϵip��ַ*/
static const char* centralized_standby_ip_recive1 = NULL;            /*���п��ƽ���վ1��ϵ��ip��ַ*/
static const char* centralized_standby_ip_send2 = NULL;            /*���п��Ʒ��͵�վ2��ϵip��ַ*/
static const char* centralized_standby_ip_recive2 = NULL;            /*���п��ƽ���վ2��ϵ��ip��ַ*/

static const char* centralized_local_ip = NULL;            /*���п��ư󶨵ı���ip��ַ*/

static uint16_t centralized_port_station1 = 0;           /*���п���վ1���ݵĶ˿ڵ�ַ*/
static uint16_t centralized_port_station2 = 0;           /*���п���վ2���ݵĶ˿ڵ�ַ*/

static CentralizedFrame self1_centralized_frame, self2_centralized_frame, peer1_centralized_frame, peer2_centralized_frame;  /*���п�������*/
#define MAX_CENTRALIZED_LENGTH 1024 /*���п��������������ͨ����*/


/*
��������    : �糡��ҵ����װ��
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
����        : 2015��9��14��
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
��������    : ���udp����
����        : 2015��9��14��
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
��������    : ���ݼ��
����        : 2015��9��14��
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
��������    : д�糡��ҵ����
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :frame--����֡      �� addr_master--��ϵ��ַ��        addr_standy--��ϵ��ַ
����        : 2015��9��14��
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
��������    : ���糡��ҵ����
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
����        : 2015��9��14��
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
��������    : ���Ϳ糡��ҵ����
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
����        : 2015��9��14��
*/
int32_t CICentralized_SendData()
{
    int32_t ret = 0;
    //ֻ����ϵ��CPU����
    if (CICpu_GetLocalState() != CPU_STATE_MASTER || CISeries_GetLocalState() != SERIES_MASTER)
    {
        return 0;
    }
    //��װ�䣬����
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
        //��װ�䣬����
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
��������    : ���տ糡��ҵ����
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
����        : 2015��9��14��
*/
int32_t CICentralized_RecvData()
{
    int32_t ret;
    //ֻ����CPU����
    if (CICpu_GetLocalState() != CPU_STATE_MASTER)
    {
        return 0;
    }
    memset(&peer1_centralized_frame, 0, sizeof(peer1_centralized_frame));
    ret = centralized_read_data(&peer1_centralized_frame,STATION1);  //��ȡ
    //CILog_Msg("test1");
    if (0 > ret)   //δ�յ�����
    {
        if (CI_FALSE == IsDoubleStation)
        {
            clear_udp_buf();
            return -1;
        }
    }
    else   //�յ�����
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
                    //����
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
        ret = centralized_read_data(&peer2_centralized_frame,STATION2);  //��ȡ
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
                        //����
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
��������    : ��ȡ���п��Ƹ��˿�
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
����        : 2015��9��14��
*/
static int32_t centralized_get_port(const char* config_name, uint16_t* port)
{
    const char* centralized_port_str = NULL;
    *port = 0;
    /*try get hmi port from configuration file*/
    centralized_port_str = CIConfig_GetValue(config_name);
    if (NULL == centralized_port_str)
    {
        CILog_Msg("�����ļ������Ҳ���%s��", config_name);
        return -1;
    }
    /*��δǿ��ת��ǰ��֤һ�ο��Ի��˿ڣ���������65535�򱨴�*/
    if (CI_FALSE == CI_ValidatePort(atoi(centralized_port_str)))
    {
        CILog_Msg("�������ļ�����%sֵ�����Ϲ淶", config_name);
        return -1;
    }
    *port = (uint16_t)atoi(centralized_port_str);
    return 0;
}

/*
��������    : ��ȡ���п��Ƹ�ip
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
����        : 2015��9��14��
*/
static const char* centralized_get_ip(const char* config_name)
{
    const char* centralized_ip_str = NULL;
    /*try get ip address from configuration file*/
    centralized_ip_str = CIConfig_GetValue(config_name);
    if (NULL == centralized_ip_str)
    {
        CILog_Msg("�����ļ������Ҳ���%s��", config_name);
        return NULL;
    }
    if (CI_FALSE == CI_ValidateIpAddress(centralized_ip_str))
    {
        CILog_Msg("�����ļ�����%s��ֵ������ip�淶", config_name);
        return NULL;
    }
    return centralized_ip_str;
}

/*
��������    : ��ʼ�����п��Ƹ�ip���˿�
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
����        : 2015��9��14��
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
��������    : ��ʼ�����п����Ƿ����վ����
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
����        : 2015��9��14��
*/
static int32_t doubleStation_init(){
    const char* double_station = NULL;
    double_station = CIConfig_GetValue("IsDoubleStation");
    if (NULL == double_station)
    {
        CILog_Msg("�����ļ������Ҳ���IsDoubleStation��");
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
        CILog_Msg("IsDoubleStation�����ô���");
        return -1;
    }

    return 0;
}


/*
��������    : �������ļ��ж��Ƿ�糡��ҵ
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :�Ƿ�糡��ҵ��־
����        : �Ų�
����        : 2015��9��14��
*/
int32_t CICentralized_IsDo(CI_BOOL *isdo)
{
    const char* cross_station = NULL;
    cross_station = CIConfig_GetValue("IsCrossStation");
    if (NULL == cross_station)
    {
        CILog_Msg("�����ļ������Ҳ���IsCrossStation��");
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
        CILog_Msg("IsCrossStation�����ô���");
        return -1;
    }
    return 0;
}

#ifdef WIN32
/*
��������    : ��windows��ʹ��socketֱ������Ի�����ͨ��
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
����        : 2015��9��21��
*/
static int32_t centralized_socket_init_windows(void)
{

    int32_t ret = 0;
    WSADATA            wsd;            //WSADATA����
    static struct sockaddr_in centralized_socket_addrlocal1;        //��������ַ
    static struct sockaddr_in centralized_socket_addrlocal2;        //��������ַ
    unsigned long ul = 1;                       /*1 block,0 nonblock*/

    //��ʼ���׽��ֶ�̬��
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        CILog_Msg("��ʼ���׽��ֶ�̬��ʧ��!\n");
        return -1;
    }

    //�����׽���
    centralized_socket_send = socket(AF_INET, SOCK_DGRAM, 0);
    if (centralized_socket_send == INVALID_SOCKET)
    {
        CILog_Msg("�����׽���ʧ�ܣ�ʧ��ԭ��; %d\n", WSAGetLastError());
        WSACleanup();//�ͷ��׽�����Դ
        return -1;
    }

    //�����׽���
    centralized_socket_from_station1 = socket(AF_INET, SOCK_DGRAM, 0);
    if (centralized_socket_from_station1 == INVALID_SOCKET)
    {
        CILog_Msg("�����׽���ʧ�ܣ�ʧ��ԭ��; %d\n", WSAGetLastError());
        WSACleanup();//�ͷ��׽�����Դ
        return -1;
    }

    //�����׽���
    centralized_socket_from_station2 = socket(AF_INET, SOCK_DGRAM, 0);
    if (centralized_socket_from_station2 == INVALID_SOCKET)
    {
        CILog_Msg("�����׽���ʧ�ܣ�ʧ��ԭ��; %d\n", WSAGetLastError());
        WSACleanup();//�ͷ��׽�����Դ
        return -1;
    }

    //��������ַ
    centralized_socket_addrlocal1.sin_family = AF_INET;
    centralized_socket_addrlocal1.sin_port = htons((short)centralized_port_station1);            //�˿�
    centralized_socket_addrlocal1.sin_addr.s_addr = inet_addr(centralized_local_ip);    //IP

    centralized_socket_addrlocal2.sin_family = AF_INET;
    centralized_socket_addrlocal2.sin_port = htons((short)centralized_port_station2);            //�˿�
    centralized_socket_addrlocal2.sin_addr.s_addr = inet_addr(centralized_local_ip);    //IP


    //���͵�ַ1
    centralized_socket_master_addrto1.sin_family = AF_INET;
    centralized_socket_master_addrto1.sin_port = htons((short)centralized_port_station1);            //�˿�
    centralized_socket_master_addrto1.sin_addr.s_addr = inet_addr(centralized_master_ip_send1);    //IP
    //���͵�ַ2
    centralized_socket_master_addrto2.sin_family = AF_INET;
    centralized_socket_master_addrto2.sin_port = htons((short)centralized_port_station2);            //�˿�
    centralized_socket_master_addrto2.sin_addr.s_addr = inet_addr(centralized_master_ip_send2);    //IP

    //��
    if (bind(centralized_socket_from_station1, (SOCKADDR *)&centralized_socket_addrlocal1, sizeof(centralized_socket_addrlocal1)) == SOCKET_ERROR)
    {
        CILog_Msg("��ʧ�ܣ�ʧ��ԭ��: %d\n", WSAGetLastError());
        closesocket(centralized_socket_from_station1);    //�ر��׽���
        WSACleanup();    //�ͷ��׽�����Դ
        return -1;
    }

    //��
    if (bind(centralized_socket_from_station2, (SOCKADDR *)&centralized_socket_addrlocal2, sizeof(centralized_socket_addrlocal2)) == SOCKET_ERROR)
    {
        CILog_Msg("��ʧ�ܣ�ʧ��ԭ��: %d\n", WSAGetLastError());
        closesocket(centralized_socket_from_station2);    //�ر��׽���
        WSACleanup();    //�ͷ��׽�����Դ
        return -1;
    }

    /*���óɷ�����ģʽ*/
    ret = ioctlsocket(centralized_socket_from_station1, FIONBIO, (unsigned long *)&ul);
    if (SOCKET_ERROR == ret)
    {
        CILog_Msg("���÷�����ʧ��");
        closesocket(centralized_socket_from_station1);
        return -1;
    }

    /*���óɷ�����ģʽ*/
    ret = ioctlsocket(centralized_socket_from_station2, FIONBIO, (unsigned long *)&ul);
    if (SOCKET_ERROR == ret)
    {
        CILog_Msg("���÷�����ʧ��");
        closesocket(centralized_socket_from_station2);
        return -1;
    }


    /*���óɷ�����ģʽ*/
    ret = ioctlsocket(centralized_socket_send, FIONBIO, (unsigned long *)&ul);
    if (SOCKET_ERROR == ret)
    {
        CILog_Msg("���÷�����ʧ��");
        closesocket(centralized_socket_send);
        return -1;
    }

    return 0;
}

#else
/*
��������    : ���п��������׽��ֳ�ʼ��
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
����        : 2015��9��14��
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
    /*Զ��ip1����*/
    bzero(&centralized_socket_master_addrto1, sizeof(struct sockaddr_in));
    centralized_socket_master_addrto1.sin_family = AF_INET;
    centralized_socket_master_addrto1.sin_addr.s_addr = inet_addr(centralized_master_ip_send1);
    centralized_socket_master_addrto1.sin_port = htons(centralized_port_station1);
    /*Զ��ip2����*/
    bzero(&centralized_socket_master_addrto2, sizeof(struct sockaddr_in));
    centralized_socket_master_addrto2.sin_family = AF_INET;
    centralized_socket_master_addrto2.sin_addr.s_addr = inet_addr(centralized_master_ip_send2);
    centralized_socket_master_addrto2.sin_port = htons(centralized_port_station2);
    /*Զ��ip3����*/
    bzero(&centralized_socket_standby_addrto1, sizeof(struct sockaddr_in));
    centralized_socket_standby_addrto1.sin_family = AF_INET;
    centralized_socket_standby_addrto1.sin_addr.s_addr = inet_addr(centralized_standby_ip_send1);
    centralized_socket_standby_addrto1.sin_port = htons(centralized_port_station1);
    /*Զ��ip4����*/
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
    /*���ؼ���ip���˿�*/
    static struct sockaddr_in centralized_socket_addrlocal_station1;
    bzero(&centralized_socket_addrlocal_station1, sizeof(struct sockaddr_in));
    centralized_socket_addrlocal_station1.sin_family = AF_INET;
    centralized_socket_addrlocal_station1.sin_addr.s_addr = inet_addr(centralized_local_ip);
    centralized_socket_addrlocal_station1.sin_port = htons(centralized_port_station1);
    /*��*/
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
    /*���ؼ���ip���˿�*/
    static struct sockaddr_in centralized_socket_addrlocal_station2;
    bzero(&centralized_socket_addrlocal_station2, sizeof(struct sockaddr_in));
    centralized_socket_addrlocal_station2.sin_family = AF_INET;
    centralized_socket_addrlocal_station2.sin_addr.s_addr = inet_addr(centralized_local_ip);
    centralized_socket_addrlocal_station2.sin_port = htons(centralized_port_station2);
    /*��*/
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
��������    : ��ʼ�����п����������
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
����        : 2015��9��14��
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

