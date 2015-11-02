/*********************************************************************
Copyright (C), 2011,  Co.Hengjun, Ltd.

����        : ZHANGB
�汾        : 1.0
��������    : 2015��10��23�� 10:42:29
��;        : ����汾����
��ʷ�޸ļ�¼:
**********************************************************************/
#include "version_manage.h"
#include "util/config.h"


#ifdef WIN32
/*win��C����������ʽʵ�ֽ�Ϊ�鷳���Լ������˼�ⷽ��*/
static CI_BOOL validateVersion(const char *version)
{
    CI_BOOL ret = CI_FALSE;
    char *p = version;
    int index = 0;
    int node_count = 0;
    while (*p != '\0')
    {
        if ((*p < '0' || *p >'9') && (*p != '.'))
            break;
        if (*p == '.')
            node_count += 1;
        p++;
        index += 1;
    }
    if ((node_count == 2) && (index == strlen(version)))
    {
        ret = CI_TRUE;
    }
    return ret;
}
#else
#include<regex.h>
#include <sys/types.h> 
/*����������ʽ��֤�汾�Ÿ�ʽ*/
static CI_BOOL validateVersion(const char *version)
{
    int err = -1;
    regex_t reg;
    regmatch_t pmatch[1];
    char *pattern = "^[0-9]+.[0-9]+.[0-9]+$";
    regcomp(&reg, pattern, REG_EXTENDED);
    err = regexec(&reg, version, 1, pmatch, 0);
    if (err == REG_NOMATCH)
    {
        regfree(&reg);
        return CI_FALSE;
    }
    regfree(&reg);
    return CI_TRUE;
}
#endif 



int32_t CIVersion_Get(char ** version)
{
    char *ret = NULL;
    ret = CIConfig_GetValue("SoftwareVersion");
    if (NULL == ret)
    {
        CILog_Msg("�����ļ������Ҳ���SoftwareVersion��");
        return -1;
    }
    else
    {
        if (validateVersion(ret) == CI_TRUE)
        {
            *version = ret;
        }
        else{
            *version = "UNKNOW";
        }
    }
    return 0;
}
