/*********************************************************************
 Copyright (C), 2014,  Co.Hengjun, Ltd.
 
 作者        : 张彦升
 版本        : 1.0
 创建日期    : 2014年8月13日 15:01:43
 用途        : 
 历史修改记录: v1.0    创建
**********************************************************************/

#include "global_data.h"

int16_t pressed_button = NO_INDEX;       /*当前按下的按钮*/
int16_t first_button = NO_INDEX;         /*当前按下的第一个按钮*/
int16_t second_button = NO_INDEX;        /*当前按下的第二个按钮*/
int16_t third_button = NO_INDEX;         /*当前按下的第三个按钮*/
int16_t fourth_button = NO_INDEX;        /*当前按下的第四个按钮*/
int16_t fifth_button = NO_INDEX;        /*当前按下的第五个按钮*/
CI_BOOL can_build_route = CI_FALSE;     /*构成进路建立命令，开始建立进路*/
CI_BOOL passing_route = CI_FALSE;       /*是否为通过进路*/
CI_BOOL FirstRunLockedSwitchSection = CI_FALSE;       /*上电启动是否锁闭道岔区段*/
int16_t wait_switches_count = 0;        /*待转换道岔数*/
int16_t switching_count = 0;            /*正在转换道岔数*/

int16_t total_buttons = 0;				/*按钮实际总数*/
int16_t total_signal_nodes = 0;		/*信号节点实际总数*/
int16_t total_ILTs = 0;				/*联锁表进路实际总数*/
int16_t total_getways = 0;				/*网关实际总数 */

int16_t total_auto_block = 0;				/*自动闭塞实际数量 */
int16_t total_auto_block3 = 0;				/*三显示自动闭塞实际数量 */
int16_t total_semi_auto_block = 0;				/*半自动闭塞实际数量 */
int16_t total_change_run_dir = 0;				/*改方运行实际数量 */
int16_t total_throat_guide_lock = 0;				/*引总锁闭实际数量 */
int16_t total_successive_route = 0;				/*延续进路实际数量 */
int16_t total_middle_switch = 0;				/*中间道岔实际数量 */
int16_t total_hold_shunting_route = 0;				/*非进路调车实际数量 */
int16_t total_special_switch = 0;				/*特殊防护道岔实际数量 */
int16_t total_switch_indication = 0;				/*同意动岔实际数量 */
int16_t total_highspeed_switch = 0;	/*提速道岔实际数量 */
int16_t total_safe_line_switch = 0;				/*安全线道岔实际数量 */
int16_t total_location_reverse = 0;				/*道岔位置相反实际数量 */
int16_t total_delay_30seconds = 0;				/*列车信号延时30s实际数量 */
int16_t total_red_filament = 0;					/*检查红灯断丝实际数量 */
int16_t total_hump = 0;				/*驼峰实际数量 */
int16_t total_request_agree = 0;				/*请求同意联系数量 */
int16_t total_yards_liaision = 0;				/*场间联系数量 */
int16_t total_state_collect = 0;				/*状态采集数量 */
int16_t total_indication_lamp = 0;				/*表示灯数量 */
int16_t time_jcbj = 20;				/*挤岔报警时间 */
int16_t total_special_input = 0;				/*特殊输入数量 */
int16_t total_special_output = 0;				/*特殊输出数量 */
int16_t total_signal_show_change = 0;				/*信号机显示变更数量 */
int16_t total_signal_delay_open = 0;				/*信号延时开放数量 */
int16_t total_section_compose = 0;				/*轨道合并数量 */
int16_t total_high_cross = 0;				/*道口数量 */
int16_t total_cross_station = 0;				/*跨场作业数量 */

char_t device_name[TOTAL_NAMES][ITEM_NAME_LENGTH];        /*设备名称存放数组*/
ST_button_desc buttons[MAX_BUTTONS];                    /*按钮所属的信号节点*/
ST_signal_node signal_nodes[MAX_SIGNAL_NODES];           /*信号节点表*/
ST_ILT_item ILT[MAX_ILTS];                               /*联锁表*/
ST_route routes[MAX_ROUTE];                               /*进路表*/
uint32_t commands[MAX_GETWAYS][MAX_EEU_PER_LAYER];       /*控制命令数据*/
ST_cleared_section clear_sections[MAX_CLEARING_SECTION];  /*出清后正在延时的区段*/
ST_switching wait_switches[MAX_WAIT_SWITCH];              /*待转换道岔表*/
ST_switching switching[MAX_CONCURRENCY_SWITCH];           /*正在转换道岔表*/
int16_t input_address[MAX_GETWAYS][MAX_EEU_PER_LAYER][MAX_NODE_PEEU]; /*输入地址映射区*/
ST_switch_alarm switch_alarm[MAX_SWITCH_JCBJ];           /*挤岔报警*/
uint16_t button_locked[MAX_BUTTON_LOCKED];           /*按钮封锁*/
int16_t train_loss_in_section[MAX_TRAIN_LOSS];     /*区段分路不良*/

auto_block_t auto_block_config[MAX_AUTO_BLOCK];             /*自动闭塞配置*/
auto_block3_t auto_block3_config[MAX_AUTO_BLOCK3];			 /*三显示自动给闭塞配置*/
semi_auto_block_t semi_auto_block_config[MAX_SEMI_AUTO_BLOCK]; /*半自动闭塞配置*/
change_dir_run_t change_run_dir_config[MAX_CHANGE_RUN_DIR]; /*改方运行配置*/
successive_t successive_route_config[MAX_SUCCESSIVE_ROUTE];  /*延续进路相关配置*/
middle_switch_t middle_switch_config[MAX_MIDLLE_SWITCH];  /*中间道岔配置*/
hold_route_shunting_t hold_route_shunting_config[MAX_HOLD_SHUNGTING_ROUTE];/*非进路调车*/
special_switch_t special_switch_config[MAX_SPECIAL_SWITCH];/*特殊防护道岔*/
agree_opreater_switch_t agree_opreater_switch_config[MAX_SWITCH_INDICATION_LAMP];/*同意动岔表示灯配置*/
highspeed_switch_t highspeed_switch_config[MAX_HIGH_SPEED_SWITCHS];/*提速道岔*/
safeline_switch_t safeline_switch_config[MAX_SAFE_LINE_SWITCHS];/*安全线道岔*/
location_reverse_t location_reverse_config[MAX_LOCATION_REVERSE];/*道岔位置相反*/
delay_30seconds_t delay_30seconds_config[MAX_DELAY_30SECONDS];/*列车信号延时30s*/
red_filament_t red_filament_config[MAX_RED_FILAMENT];/*检查红灯断丝*/

request_agree request_agree_config[MAX_REQUEST_AGREE];/*请求同意配置*/
yards_liaision yards_liaision_config[MAX_YARDS_LIAISION];/*场间联系配置*/
state_collect_t state_collect_config[MAX_STATE_COLLECT];/*状态采集配置*/
indication_lamp indication_lamp_config[MAX_INDICATION_LAMP];/*表示灯配置*/


other_special_t other_special_config;/*其他特殊配置*/

dk_t dk_config[MAX_HIGH_CROSS];/*普通道口*/
dk_bgfylz65_t dk_bgfylz65_config;/*宝钢付原料站65#道口*/
dk_bgcpkz53_t dk_bgcpkz53_config;/*宝钢成品库站53#道口*/
special_input_t special_input_config[MAX_SPECIAL_INPUT];/*特殊输入配置*/
special_input_t special_output_config[MAX_SPECIAL_INPUT];/*特殊输出配置*/
signal_show_change_t signal_show_change_config[MAX_SIGNAL_SHOW_CHANGE];/*信号机显示变更配置*/
signa_delay_open_t signal_delay_open_config[MAX_SIGNAL_DELAY_OPEN];/*信号延时开放配置*/
signal_show_change_t signal_show_change_config[MAX_SIGNAL_SHOW_CHANGE];/*信号机显示变更配置*/
section_compose_t section_compose_config[MAX_SECTION_COMPOSE];/*轨道合并配置*/

//cross_station_t send_cross_station_config[MAX_CROSS_STATION];/*跨场作业发送数据*/
//cross_station_t recv1_cross_station_config[MAX_CROSS_STATION];/*跨场作业接收数据1*/
//cross_station_t recv2_cross_station_config[MAX_CROSS_STATION];/*跨场作业接收数据2*/


/**************************************************
跨场作业说明：
1.SendToStart表示建立进路时前方进路状态；
2.SendToEnd表示是否折返；
3.RecviceFromStart表示接收折返信息；
4.RecviceFromEnd表示接收前方进路状态。
注：此数据仅针对宝钢一站三场跨场作业设置，其他站场
    如有需要，请谨慎使用。
		  工厂站
		 /		\
		/		 \
	付原料站  成品库站
**************************************************/
/*站场1*/
cross_station_end_start_t CrossStation1SendToStart[MAX_CROSS_STATION];/*跨场作业终端-->始端发送数据*/
cross_station_end_start_t CrossStation1RecviceFromEnd[MAX_CROSS_STATION];/*跨场作业接收终端数据*/
cross_station_start_end_t CrossStation1SendToEnd[MAX_CROSS_STATION];/*跨场作业始端-->终端发送数据*/
cross_station_start_end_t CrossStation1RecviceFromStart[MAX_CROSS_STATION];/*跨场作业接收始端数据*/
/*站场2*/
cross_station_end_start_t CrossStation2SendToStart[MAX_CROSS_STATION];/*跨场作业终端-->始端发送数据*/
cross_station_end_start_t CrossStation2RecviceFromEnd[MAX_CROSS_STATION];/*跨场作业接收终端数据*/
cross_station_start_end_t CrossStation2SendToEnd[MAX_CROSS_STATION];/*跨场作业始端-->终端发送数据*/
cross_station_start_end_t CrossStation2RecviceFromStart[MAX_CROSS_STATION];/*跨场作业接收始端数据*/
/*虚拟进路*/
cross_station_route_t CrossStationRoute[MAX_CROSS_STATION];/*跨场作业虚拟进路数据*/
