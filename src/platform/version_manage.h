/*********************************************************************
Copyright (C), 2011,  Co.Hengjun, Ltd.

用途        : 软件版本管理
历史修改记录:
**********************************************************************/
#ifndef _version_manage_h__
#define _version_manage_h__

#include "util/ci_header.h"
/*
 *获取软件版本号
 *参数：获取的版本号
 *返回值：成功为0 ， 失败为-1
*/
CIAPI_FUNC(int32_t) CIVersion_Get(char ** version);
#endif