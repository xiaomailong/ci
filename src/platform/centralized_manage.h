/*********************************************************************
Copyright (C), 2011,  Co.Hengjun, Ltd.

��;        : ���п��ƹ���
��ʷ�޸ļ�¼:
**********************************************************************/
#ifndef _centralized_manage_h__
#define _centralized_manage_h__

#include "util/ci_header.h"

/*
��������    : ���Ϳ糡��ҵ����
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
����        : �Ų�
����        : 2015��9��14��
*/
CIAPI_FUNC(int32_t) CICentralized_SendData(void);

/*
��������    : ���տ糡��ҵ����
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
����        : �Ų�
����        : 2015��9��14��
*/
CIAPI_FUNC(int32_t) CICentralized_RecvData(void);

/*
��������    : �������ļ��ж��Ƿ�糡��ҵ
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :�Ƿ�糡��ҵ��־
����        : �Ų�
����        : 2015��9��14��
*/
CIAPI_FUNC(int32_t) CICentralized_IsDo(CI_BOOL *isdo);

/*
��������    : ��ʼ�����п����������
����ֵ      : �ɹ�Ϊ0��ʧ��Ϊ-1
����        :
����        : �Ų�
����        : 2015��9��14��
*/
CIAPI_FUNC(int32_t) CICentralized_Init(void);

#endif /*_centralized_manage_h__*/