/***************************************************************************************
Copyright (C), 2015,  Co.Hengjun, Ltd.
文件名:special_interlocking.c
作者:  hejh
版本:  V1.0	
日期:  2015/07/27
用途:  特殊联锁模块
历史修改记录:  

***************************************************************************************/
#ifndef SPECIAL_INTERLOCKING
#define SPECIAL_INTERLOCKING

#include "global_data.h"
#include "utility_function.h"
#include "error_process.h"
#include "util/ci_header.h"
#include "base_type_definition.h"
#include "keep_signal.h"
#include "auto_unlock.h"

/****************************************************
函数名：   sepecial_interlocking
功能描述： 特殊联锁
返回值：   void
作者：	   hejh
日期：     2015/07/27
****************************************************/
void sepecial_interlocking();

/****************************************************
函数名：   dk
功能描述： 普通道口
返回值：   void
作者：	   hejh
日期：     2015/08/15
****************************************************/
void dk();

/****************************************************
函数名：   dk_bgfylz65
功能描述： 宝钢付原料站65#道口
返回值：   void
参数：     int16_t button_index
作者：	   hejh
日期：     2015/07/27
****************************************************/
void dk_bgfylz65(int16_t button_index);

/****************************************************
函数名：   dk_bgcpkz53
功能描述： 宝钢成品库站53#道口
返回值：   void
作者：	   hejh
日期：     2015/08/13
****************************************************/
void dk_bgcpkz53();

/****************************************************
函数名：   cross_station
功能描述： 跨场作业
返回值：   void
作者：	   hejh
日期：     2015/09/11
****************************************************/
void cross_station();

/****************************************************
函数名：   cross_station_check_all_unlock_condition
功能描述： 跨场折返检查全部未解锁进路条件
返回值：   route_t
参数：     route_t route_index
作者：	   hejh
日期：     2015/10/20
****************************************************/
route_t cross_station_check_all_unlock_condition(route_t route_index);

/****************************************************
函数名：   cross_station_check_part_unlock_condition
功能描述： 跨场折返检查部分未解锁进路条件
返回值：   CI_BOOL
参数：     route_t route_index
作者：	   hejh
日期：     2015/10/15
****************************************************/
CI_BOOL cross_station_check_part_unlock_condition(route_t route_index);

/****************************************************
函数名：   cross_station_search_return_signal
功能描述： 搜索跨场折返信号机
返回值：   int16_t
参数：     route_t route_index
参数：     CI_BOOL is_all_unlock_route
作者：	   hejh
日期：     2015/10/20
****************************************************/
int16_t cross_station_search_return_signal(route_t route_index,CI_BOOL is_all_unlock_route);

/****************************************************
函数名：   cross_station_is_switch_location_right
功能描述： 跨场折返道岔位置正确
返回值：   node_t
参数：     route_t current_route
参数：     node_t current_node
参数：     index_t current_ordinal
作者：	   hejh
日期：     2015/10/15
****************************************************/
node_t cross_station_is_switch_location_right(route_t current_route,node_t current_node,index_t current_ordinal);

/****************************************************
函数名：   cross_station_function
功能描述： 跨场作业虚拟进路功能操作
返回值：   CI_BOOL
参数：     CI_BOOL delay_unlock_flag
作者：	   hejh
日期：     2015/10/21
****************************************************/
CI_BOOL cross_station_function(CI_BOOL delay_unlock_flag);

/****************************************************
函数名：   cross_station_route
功能描述： 跨场作业虚拟进路关闭信号和区段解锁
返回值：   void
作者：	   hejh
日期：     2015/10/22
****************************************************/
void cross_station_route();

/****************************************************
函数名：   cross_station_init
功能描述： 跨场作业进路状态初始化
返回值：   void
作者：	   hejh
日期：     2015/11/03
****************************************************/
void cross_station_init();

#endif