#ifndef _centralized_syn_h__
#define _centralized_syn_h__

#include "util/ci_header.h"
//#include "interlocking/base_type_definition.h"
#include "interlocking/data_struct_definition.h"

typedef  cross_station_end_start_t  CentralizedToStartData;
typedef  cross_station_start_end_t  CentralizedToEndData;

/*跨场作业数据结构*/
typedef struct _CentralizedData
{
    CentralizedToStartData centralized_to_start_data[MAX_CROSS_STATION];  /*跨场作业进路数据*/
    CentralizedToEndData centralized_to_end_data[MAX_CROSS_STATION];   /*跨场作业折返数据*/
}CentralizedData;

/*跨场作业数据帧*/
typedef struct _CentralizedFrame
{
    uint16_t frame_head_tag;                /*帧头标志*/
    uint16_t frame_type;                    /*帧类型*/
    uint16_t frame_sn;                      /*帧编号*/
    CentralizedData centralizedData;       /*跨场作业数据*/
    uint16_t crc;                            /*crc校验码*/
}CentralizedFrame;

/*
功能描述    : 装配集中控制数据帧
返回值      : 成功为0，失败为-1
参数        :
作者        : 张博
日期        : 2015年9月14日
*/
CIAPI_FUNC(int32_t) CICentralizedSyn1_Assemble(CentralizedFrame* p_frame);

/*
功能描述    : 装配集中控制数据帧
返回值      : 成功为0，失败为-1
参数        :
作者        : 张博
日期        : 2015年9月14日
*/
CIAPI_FUNC(int32_t) CICentralizedSyn2_Assemble(CentralizedFrame* p_frame);

/*
功能描述    : 校验集中控制相关数据
返回值      : 成功为0，失败为-1
参数        :
作者        : 张博
日期        : 2015年9月14日
*/
CIAPI_FUNC(CI_BOOL) CICentralizedSyn_Verify(CentralizedFrame *p_frame);

/*
功能描述    : 解析集中控制相关数据1
返回值      : 成功为0，失败为-1
参数        :
作者        : 张博
日期        : 2015年9月14日
*/
CIAPI_FUNC(int32_t) CICentralizedSyn1_Analysise(CentralizedFrame* p_frame);

/*
功能描述    : 解析集中控制相关数据2
返回值      : 成功为0，失败为-1
参数        :
作者        : 张博
日期        : 2015年9月14日
*/
CIAPI_FUNC(int32_t) CICentralizedSyn2_Analysise(CentralizedFrame* p_frame);

/*
功能描述    : 复制数据帧
返回值      : 成功为0，失败为-1
参数        :
作者        : 张博
日期        : 2015年9月14日
*/
CIAPI_FUNC(int32_t) CICentralizedSyn_CopyTo(CentralizedFrame* src_frame, CentralizedFrame* dst_frame);

#endif