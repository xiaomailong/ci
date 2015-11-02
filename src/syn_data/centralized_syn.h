#ifndef _centralized_syn_h__
#define _centralized_syn_h__

#include "util/ci_header.h"
//#include "interlocking/base_type_definition.h"
#include "interlocking/data_struct_definition.h"

typedef  cross_station_end_start_t  CentralizedToStartData;
typedef  cross_station_start_end_t  CentralizedToEndData;

/*�糡��ҵ���ݽṹ*/
typedef struct _CentralizedData
{
    CentralizedToStartData centralized_to_start_data[MAX_CROSS_STATION];  /*�糡��ҵ��·����*/
    CentralizedToEndData centralized_to_end_data[MAX_CROSS_STATION];   /*�糡��ҵ�۷�����*/
}CentralizedData;

/*�糡��ҵ����֡*/
typedef struct _CentralizedFrame
{
    uint16_t frame_head_tag;                /*֡ͷ��־*/
    uint16_t frame_type;                    /*֡����*/
    uint16_t frame_sn;                      /*֡���*/
    CentralizedData centralizedData;       /*�糡��ҵ����*/
    uint16_t crc;                            /*crcУ����*/
}CentralizedFrame;

/*
��������    : װ�伯�п�������֡
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
����        : �Ų�
����        : 2015��9��14��
*/
CIAPI_FUNC(int32_t) CICentralizedSyn1_Assemble(CentralizedFrame* p_frame);

/*
��������    : װ�伯�п�������֡
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
����        : �Ų�
����        : 2015��9��14��
*/
CIAPI_FUNC(int32_t) CICentralizedSyn2_Assemble(CentralizedFrame* p_frame);

/*
��������    : У�鼯�п����������
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
����        : �Ų�
����        : 2015��9��14��
*/
CIAPI_FUNC(CI_BOOL) CICentralizedSyn_Verify(CentralizedFrame *p_frame);

/*
��������    : �������п����������1
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
����        : �Ų�
����        : 2015��9��14��
*/
CIAPI_FUNC(int32_t) CICentralizedSyn1_Analysise(CentralizedFrame* p_frame);

/*
��������    : �������п����������2
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
����        : �Ų�
����        : 2015��9��14��
*/
CIAPI_FUNC(int32_t) CICentralizedSyn2_Analysise(CentralizedFrame* p_frame);

/*
��������    : ��������֡
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
����        : �Ų�
����        : 2015��9��14��
*/
CIAPI_FUNC(int32_t) CICentralizedSyn_CopyTo(CentralizedFrame* src_frame, CentralizedFrame* dst_frame);

#endif