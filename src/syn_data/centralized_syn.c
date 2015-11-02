#include "interlocking/global_data.h"
#include "centralized_syn.h"
#include "interlocking/inter_api.h"

/*
��������    : crcУ��
����ֵ      : 
����        :
*/
static uint16_t centralizedSyn_crc(CentralizedFrame* p_frame)
{
    uint16_t old_hash = p_frame->crc;
    uint16_t crc_code = 0;
    p_frame->crc = 0;
    crc_code = CIAlgorithm_Crc16(p_frame, sizeof(CentralizedFrame));
    p_frame->crc = crc_code;
    return old_hash;
}

/*
��������    : װ�伯�п�������֡
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
*/
int32_t CICentralizedSyn1_Assemble(CentralizedFrame* p_frame)
{
    register int i = 0;
    register int j = 0;

    p_frame->frame_head_tag = 0xaa55;
    p_frame->frame_type = 0x01;
    p_frame->frame_sn = CICycleInt_GetCounter();

    /*װ��糡��ҵ����*/
    for (i = 0; i < MAX_CROSS_STATION; i++)
    {
        p_frame->centralizedData.centralized_to_start_data[i].StartSignal = CrossStation1SendToStart[i].StartSignal;
        p_frame->centralizedData.centralized_to_start_data[i].RouteState = CrossStation1SendToStart[i].RouteState;
        for (j = 0; j < TEST_NAME_LENGTH; j ++)
        {
            p_frame->centralizedData.centralized_to_start_data[i].LinkSignal[j] = CrossStation1SendToStart[i].LinkSignal[j];
            p_frame->centralizedData.centralized_to_end_data[i].LinkSignal[j] = CrossStation1SendToEnd[i].LinkSignal[j];
        }       
        p_frame->centralizedData.centralized_to_end_data[i].StartSignal = CrossStation1SendToEnd[i].StartSignal;
        p_frame->centralizedData.centralized_to_end_data[i].IsRetun = CrossStation1SendToEnd[i].IsRetun;
        p_frame->centralizedData.centralized_to_end_data[i].LockSection = CrossStation1SendToEnd[i].LockSection;

        p_frame->centralizedData.centralized_to_end_data[i].RouteDirection = CrossStation1SendToEnd[i].RouteDirection;
        p_frame->centralizedData.centralized_to_end_data[i].SectionState = CrossStation1SendToEnd[i].SectionState;
    }
    /*����crc*/
    centralizedSyn_crc(p_frame);
    return 0;
}

/*
��������    : װ�伯�п�������֡2
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
*/
int32_t CICentralizedSyn2_Assemble(CentralizedFrame* p_frame)
{
    register int i = 0;
    register int j = 0;

    p_frame->frame_head_tag = 0xaa55;
    p_frame->frame_type = 0x02;
    p_frame->frame_sn = CICycleInt_GetCounter();

    /*װ��糡��ҵ����*/
    for (i = 0; i < MAX_CROSS_STATION; i++)
    {
        p_frame->centralizedData.centralized_to_start_data[i].StartSignal = CrossStation2SendToStart[i].StartSignal;
        p_frame->centralizedData.centralized_to_start_data[i].RouteState = CrossStation2SendToStart[i].RouteState;
        for (j = 0; j < TEST_NAME_LENGTH; j++)
        {
            p_frame->centralizedData.centralized_to_start_data[i].LinkSignal[j] = CrossStation2SendToStart[i].LinkSignal[j];
            p_frame->centralizedData.centralized_to_end_data[i].LinkSignal[j] = CrossStation2SendToEnd[i].LinkSignal[j];
        }
        p_frame->centralizedData.centralized_to_end_data[i].StartSignal = CrossStation2SendToEnd[i].StartSignal;
        p_frame->centralizedData.centralized_to_end_data[i].IsRetun = CrossStation2SendToEnd[i].IsRetun;
        p_frame->centralizedData.centralized_to_end_data[i].LockSection = CrossStation2SendToEnd[i].LockSection;

        p_frame->centralizedData.centralized_to_end_data[i].RouteDirection = CrossStation2SendToEnd[i].RouteDirection;
        p_frame->centralizedData.centralized_to_end_data[i].SectionState = CrossStation2SendToEnd[i].SectionState;
    }
    /*����crc*/
    centralizedSyn_crc(p_frame);
    return 0;
}

/*
��������    : ��������֡
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
*/
int32_t CICentralizedSyn_CopyTo(CentralizedFrame* src_frame, CentralizedFrame* dst_frame)
{
    register int i = 0;
    register int j = 0;

    dst_frame->frame_head_tag = src_frame->frame_head_tag;
    dst_frame->frame_type = src_frame->frame_type;
    dst_frame->frame_sn = src_frame->frame_sn;

    /*���ƿ糡��ҵ����*/
    for (i = 0; i < MAX_CROSS_STATION; i++)
    {
        dst_frame->centralizedData.centralized_to_start_data[i].StartSignal = src_frame->centralizedData.centralized_to_start_data[i].StartSignal;
        dst_frame->centralizedData.centralized_to_start_data[i].RouteState = src_frame->centralizedData.centralized_to_start_data[i].RouteState;
        for (j = 0; j < TEST_NAME_LENGTH; j++)
        {
            dst_frame->centralizedData.centralized_to_start_data[i].LinkSignal[j] = src_frame->centralizedData.centralized_to_start_data[i].LinkSignal[j];
            dst_frame->centralizedData.centralized_to_end_data[i].LinkSignal[j] = src_frame->centralizedData.centralized_to_end_data[i].LinkSignal[j];
        }
        dst_frame->centralizedData.centralized_to_end_data[i].StartSignal = src_frame->centralizedData.centralized_to_end_data[i].StartSignal;
        dst_frame->centralizedData.centralized_to_end_data[i].IsRetun = src_frame->centralizedData.centralized_to_end_data[i].IsRetun;
        dst_frame->centralizedData.centralized_to_end_data[i].LockSection = src_frame->centralizedData.centralized_to_end_data[i].LockSection;

        dst_frame->centralizedData.centralized_to_end_data[i].RouteDirection = src_frame->centralizedData.centralized_to_end_data[i].RouteDirection;
        dst_frame->centralizedData.centralized_to_end_data[i].SectionState = src_frame->centralizedData.centralized_to_end_data[i].SectionState;
        
    }

    dst_frame->crc = src_frame->crc;
    return 0;
}

/*
��������    : У�鼯�п�������
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
*/
CI_BOOL CICentralizedSyn_Verify(CentralizedFrame *p_frame)
{
    uint16_t old_hash = 0;
    /*�������У�����Ƿ���ȷ*/
    old_hash = centralizedSyn_crc(p_frame);
    if (old_hash != p_frame->crc)
    {
        return CI_FALSE;
    }
    return CI_TRUE;
}

/*
��������    : �������п�������֡1
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
*/
int32_t CICentralizedSyn1_Analysise(CentralizedFrame* p_frame)
{
    register int i = 0;
    register int j = 0;

    /*�����糡��ҵ����*/
    for (i = 0; i < MAX_CROSS_STATION; i++)
    {
        CrossStation1RecviceFromEnd[i].StartSignal = p_frame->centralizedData.centralized_to_start_data[i].StartSignal;
        CrossStation1RecviceFromEnd[i].RouteState = p_frame->centralizedData.centralized_to_start_data[i].RouteState;
        for (j = 0; j < TEST_NAME_LENGTH; j++)
        {
            CrossStation1RecviceFromEnd[i].LinkSignal[j] = p_frame->centralizedData.centralized_to_start_data[i].LinkSignal[j];
            CrossStation1RecviceFromStart[i].LinkSignal[j] = p_frame->centralizedData.centralized_to_end_data[i].LinkSignal[j];
        }
        CrossStation1RecviceFromStart[i].StartSignal = p_frame->centralizedData.centralized_to_end_data[i].StartSignal;
        CrossStation1RecviceFromStart[i].IsRetun = p_frame->centralizedData.centralized_to_end_data[i].IsRetun;
        CrossStation1RecviceFromStart[i].LockSection = p_frame->centralizedData.centralized_to_end_data[i].LockSection;

        CrossStation1RecviceFromStart[i].RouteDirection = p_frame->centralizedData.centralized_to_end_data[i].RouteDirection;
        CrossStation1RecviceFromStart[i].SectionState = p_frame->centralizedData.centralized_to_end_data[i].SectionState;
    }
    return 0;
}

/*
��������    : �������п�������֡2
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
*/
int32_t CICentralizedSyn2_Analysise(CentralizedFrame* p_frame)
{
    register int i = 0;
    register int j = 0;

    /*�����糡��ҵ����*/
    for (i = 0; i < MAX_CROSS_STATION; i++)
    {
        CrossStation2RecviceFromEnd[i].StartSignal = p_frame->centralizedData.centralized_to_start_data[i].StartSignal;
        CrossStation2RecviceFromEnd[i].RouteState = p_frame->centralizedData.centralized_to_start_data[i].RouteState;
        for (j = 0; j < TEST_NAME_LENGTH; j++)
        {
            CrossStation2RecviceFromEnd[i].LinkSignal[j] = p_frame->centralizedData.centralized_to_start_data[i].LinkSignal[j];
            CrossStation2RecviceFromStart[i].LinkSignal[j] = p_frame->centralizedData.centralized_to_end_data[i].LinkSignal[j];
        }
        CrossStation2RecviceFromStart[i].StartSignal = p_frame->centralizedData.centralized_to_end_data[i].StartSignal;
        CrossStation2RecviceFromStart[i].IsRetun = p_frame->centralizedData.centralized_to_end_data[i].IsRetun;
        CrossStation2RecviceFromStart[i].LockSection = p_frame->centralizedData.centralized_to_end_data[i].LockSection;

        CrossStation2RecviceFromStart[i].RouteDirection = p_frame->centralizedData.centralized_to_end_data[i].RouteDirection;
        CrossStation2RecviceFromStart[i].SectionState = p_frame->centralizedData.centralized_to_end_data[i].SectionState;
    }
    return 0;
}




