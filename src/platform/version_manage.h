/*********************************************************************
Copyright (C), 2011,  Co.Hengjun, Ltd.

��;        : ����汾����
��ʷ�޸ļ�¼:
**********************************************************************/
#ifndef _version_manage_h__
#define _version_manage_h__

#include "util/ci_header.h"
/*
 *��ȡ����汾��
 *��������ȡ�İ汾��
 *����ֵ���ɹ�Ϊ0 �� ʧ��Ϊ-1
*/
CIAPI_FUNC(int32_t) CIVersion_Get(char ** version);
#endif