/*********************************************************************
Copyright (C), 2011,  Co.Hengjun, Ltd.

用途        : 集中控制管理
历史修改记录:
**********************************************************************/
#ifndef _centralized_manage_h__
#define _centralized_manage_h__

#include "util/ci_header.h"

/*
功能描述    : 发送跨场作业数据
返回值      : 成功为0，失败为-1
参数        :
作者        : 张博
日期        : 2015年9月14日
*/
CIAPI_FUNC(int32_t) CICentralized_SendData(void);

/*
功能描述    : 接收跨场作业数据
返回值      : 成功为0，失败为-1
参数        :
作者        : 张博
日期        : 2015年9月14日
*/
CIAPI_FUNC(int32_t) CICentralized_RecvData(void);

/*
功能描述    : 从配置文件判断是否跨场作业
返回值      : 成功为0，失败为-1
参数        :是否跨场作业标志
作者        : 张博
日期        : 2015年9月14日
*/
CIAPI_FUNC(int32_t) CICentralized_IsDo(CI_BOOL *isdo);

/*
功能描述    : 初始化集中控制相关数据
返回值      : 成功为0，失败为-1
参数        :
作者        : 张博
日期        : 2015年9月14日
*/
CIAPI_FUNC(int32_t) CICentralized_Init(void);

#endif /*_centralized_manage_h__*/