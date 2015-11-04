﻿/***************************************************************************************
Copyright (C), 2011,  Co.Hengjun, Ltd.
文件名:  process_command.h
作者:    CY
版本 :   1.0	
创建日期:2011/11/30
用途:    命令处理模块
历史修改记录:         
***************************************************************************************/
#ifndef PROCESS_COMMAND
#define PROCESS_COMMAND

#include "base_type_definition.h"

#include "util/ci_header.h"

/****************************************************
函数名:    process_command
功能描述:  命令处理模块
返回值:    
参数:      
作者  :    CY
日期  ：   2011/11/30
****************************************************/
void process_command(void);

/****************************************************
函数名:    single_button_command
功能描述:  判断已接收到的第一个按钮是否直接形成命令，
		如果形成命令，直接调用相应的命令模块。
返回值:		
作者  :    LYC
日期  ：   2011/11/29
****************************************************/
void single_button_command(void);	

/****************************************************
函数名:    is_build_route_command
功能描述:  判断已接收到的两个按钮是否可以构成建立
		 进路的命令。
返回值:  如果可以建立进路返回真，无法建立进路返回假
作者  :    LYC
日期  ：   2011/11/29
****************************************************/
CI_BOOL is_build_route_command(void);

/****************************************************
函数名:    is_three_button_build_route
功能描述:  判断三按钮变更进路建立条件
返回值:		如果可以建立进路返回真，无法建立进路返回假
作者  :    LYC
日期  ：   2011/12/1
****************************************************/
CI_BOOL is_three_button_build_route(void);

/****************************************************
函数名:     is_four_button_build_route
功能描述:   判断四按钮变更进路建立条件
返回值:		如果可以建立进路返回真，无法建立进路返回假
作者  :    LYC
日期  ：   2011/12/1
****************************************************/
CI_BOOL is_four_button_build_route();

/****************************************************
函数名：   is_five_button_build_route
功能描述： 判断5个按钮的进路建立条件
返回值：   CI_BOOL
参数：     void
作者：	   hejh
日期：     2015/06/02
****************************************************/
CI_BOOL  is_five_button_build_route(void);

/****************************************************
函数名:    clean_button_log
功能描述:  清除四个按钮的记录
返回值:	   无
作者  :    LYC
日期  ：   2011/11/29
****************************************************/
void clean_button_log(void);

/****************************************************
函数名:    function_button_choose
功能描述:  根据第一个按下的功能按钮选择调用相应的模块
返回值:    无
作者  :    LYC
日期  ：   2011/11/29
****************************************************/
void function_button_choose(void);

/****************************************************
函数名:    process_third_button
功能描述:  第三个按钮处理
返回类值:  无
作者  :    LYC
日期  ：   2011/11/29
****************************************************/
void process_third_button(void);

/****************************************************
函数名:    need_third_button
功能描述:  根据当前按下的两个按钮判断是否还需按下
第三个按钮才能形成命令
返回值:		需要按下第三个按钮返回真，不需要返回假
作者  :    LYC
日期  ：   2011/12/2
****************************************************/
CI_BOOL need_third_button(void);

/*
 功能描述    : 平台层要使用运算层当中使用的button信息，通过此函数获得
 返回值      : 记录的当前按下的按钮pressed_button
 参数        : 
 作者        : 何境泰
 日期        : 2014年6月20日 12:40:09
*/
CIAPI_FUNC(int16_t) CICommand_GetButton(int16_t *button1,
                                        int16_t *button2, 
                                        int16_t *button3,
                                        int16_t *button4,
                                        int16_t *button5);
/*
 功能描述    : 对按钮信息进行更改
 返回值      : 记录的当前按下的按钮pressed_button
 参数        : 
 作者        : 何境泰
 日期        : 2014年6月20日 12:40:09
*/
CIAPI_FUNC(int16_t) CICommand_SetButton(int16_t button1,
                                        int16_t button2, 
                                        int16_t button3,
                                        int16_t button4,
                                        int16_t button5);


#endif
