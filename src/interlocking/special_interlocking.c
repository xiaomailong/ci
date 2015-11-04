/***************************************************************************************
Copyright (C), 2015,  Co.Hengjun, Ltd.
文件名:special_interlocking.c
作者:  hejh
版本:  V1.0	
日期:  2015/07/27
用途:  特殊联锁模块
历史修改记录:  

***************************************************************************************/
#include "special_interlocking.h"

/****************************************************
函数名：   sepecial_interlocking
功能描述： 特殊联锁
返回值：   void
作者：	   hejh
日期：     2015/07/27
****************************************************/
void sepecial_interlocking()
{
	dk();

	dk_bgfylz65(NO_INDEX);

	dk_bgcpkz53();
	
	
	cross_station_route();	
	cross_station_function(CI_TRUE);	
	cross_station();

}

/****************************************************
函数名：   dk
功能描述： 普通道口
返回值：   void
作者：	   hejh
日期：     2015/08/15
****************************************************/
void dk()
{
	int16_t i,j,k;
	route_t route_index = NO_INDEX;
	CI_BOOL result = CI_FALSE,add_flag = CI_TRUE;
	int16_t dk[MAX_HIGH_CROSS],dk_count = 0;

	for (i = 0; i < TOTAL_HIGH_CROSS; i++)
	{
		if (dk_config[i].AlarmIndex != NO_INDEX)
		{
			/*道口报警状态*/
			if (IsTRUE(dk_config[i].AlarmState))
			{
				/*停止报警*/
				if ((dk_config[i].StopSection != NO_INDEX)
					&& (gn_section_state(dk_config[i].StopSection) == SCS_CLEARED)
					&& (IsFALSE(is_node_locked(dk_config[i].StopSection,LT_LOCKED))))
				{
					dk_config[i].AlarmState = CI_FALSE;
				}
			}
			/*道口非报警状态*/
			else
			{
				/*检查信号机*/
				if (dk_config[i].SignalIndex != NO_INDEX)
				{
					route_index = gn_belong_route(dk_config[i].SignalIndex);
					if (route_index != NO_INDEX)
					{
						/*进路类型正确*/
						if (((dk_config[i].RouteType == RT_SHUNTING_ROUTE) && (gr_type(route_index) == RT_SHUNTING_ROUTE))
							|| ((dk_config[i].RouteType == RT_TRAIN_ROUTE) && (gr_type(route_index) == RT_TRAIN_ROUTE))
							|| (dk_config[i].RouteType == RT_ERROR))
						{
							/*报警条件*/
							if ((IsFALSE(is_signal_close(dk_config[i].SignalIndex)))
								&& (dk_config[i].StartSection != NO_INDEX)
								&& (gn_section_state(dk_config[i].StartSection) != SCS_CLEARED))
							{
								result = CI_TRUE;
								for (j = 0; j < dk_config[i].SwitchsCount; j++)
								{
									/*道岔位置符合本进路*/
									for (k = 0; k < gr_switches_count(route_index); k++)
									{
										if (dk_config[i].Switchs[j].SwitchIndex == gr_switch(route_index,k))
										{
											if (dk_config[i].Switchs[j].Location != gr_switch_location(route_index,k))
											{
												result = CI_FALSE;
												break;
											}
										}
									}
								}
								if (IsTRUE(result))
								{
									dk_config[i].AlarmState = CI_TRUE;
								}
							}
						}
					}
				}
				/*不检查信号机*/
				else
				{
					/*报警条件*/
					if ((dk_config[i].StartSection != NO_INDEX)
						&& (gn_section_state(dk_config[i].StartSection) != SCS_CLEARED)
						&& (dk_config[i].StopSection != NO_INDEX)
						&& (IsTRUE(is_node_locked(dk_config[i].StopSection,LT_LOCKED))))
					{
						result = CI_TRUE;
						route_index = gn_belong_route(dk_config[i].StopSection);
						if (route_index != NO_INDEX)
						{
							for (j = 0; j < TOTAL_HIGH_CROSS; j++)
							{
								if ((dk_config[j].SignalIndex != NO_INDEX)
									&& (dk_config[j].SignalIndex == gr_start_signal(route_index))
									&& (gn_belong_route(dk_config[j].SignalIndex) == route_index))
								{
									result = CI_FALSE;
									break;
								}
							}
						}

						if (IsTRUE(result))
						{
							for (j = 0; j < dk_config[i].SwitchsCount; j++)
							{
								/*道岔位置符合本进路*/
								for (k = 0; k < gr_switches_count(route_index); k++)
								{
									if (dk_config[i].Switchs[j].SwitchIndex == gr_switch(route_index,k))
									{
										if (dk_config[i].Switchs[j].Location != gr_switch_location(route_index,k))
										{
											result = CI_FALSE;
											break;
										}
									}
								}
							}
						}						
						if (IsTRUE(result))
						{
							dk_config[i].AlarmState = CI_TRUE;
						}
					}
				}
			}
		}		
	}

	/*统计道口输出数量*/
	for (i = 0; i < TOTAL_HIGH_CROSS; i++)
	{
		add_flag = CI_TRUE;
		for (j = 0; j < dk_count; j++)
		{
			if (dk[j] == dk_config[i].AlarmIndex)
			{
				add_flag = CI_FALSE;
				break;
			}
		}
		if (IsTRUE(add_flag))
		{
			dk[dk_count] = dk_config[i].AlarmIndex;
			dk_count++;
		}		
	}
	/*输出*/
	for (i = 0; i < dk_count; i++)
	{
		result = CI_FALSE;
		for (j = 0; j < TOTAL_HIGH_CROSS; j++)
		{
			if ((dk[i] == dk_config[j].AlarmIndex)
				&& (IsTRUE(dk_config[j].AlarmState)))
			{
				result = CI_TRUE;
				break;
			}
		}
		if (IsTRUE(result))
		{
			send_command(dk[i],SIO_NO_SIGNAL);
		}
		else
		{
			send_command(dk[i],SIO_HAS_SIGNAL);
		}
	}
}

/****************************************************
函数名：   dk_bgfylz65
功能描述： 宝钢付原料站65#道口
返回值：   void
参数：     int16_t button_index
作者：	   hejh
日期：     2015/07/27
****************************************************/
void dk_bgfylz65(int16_t button_index)
{
	if (button_index != NO_INDEX)
	{
		if (dk_bgfylz65_config.section1 != NO_INDEX)
		{
			/*按下同意按钮*/
			if (gb_node(button_index) == dk_bgfylz65_config.TYD)
			{
				send_command(dk_bgfylz65_config.TYD,SIO_HAS_SIGNAL);
				dk_bgfylz65_config.send_cycle_count = CICycleInt_GetCounter();
			}

			/*按下收权按钮*/
			if (gb_node(button_index) == dk_bgfylz65_config.SQD)
			{
				send_command(dk_bgfylz65_config.SQD,SIO_HAS_SIGNAL);
				dk_bgfylz65_config.send_cycle_count = CICycleInt_GetCounter();
			}
		}
	}
	else
	{
		if (dk_bgfylz65_config.section1 != NO_INDEX)
		{
			///*停止输出同意信息*/
			//if (gn_state(dk_bgfylz65_config.TYD) == SIO_HAS_SIGNAL)
			//{
			//	send_command(dk_bgfylz65_config.TYD,SIO_NO_SIGNAL);
			//}
			///*停止输出收权信息*/
			//if (gn_state(dk_bgfylz65_config.SQD) == SIO_HAS_SIGNAL)
			//{
			//	send_command(dk_bgfylz65_config.SQD,SIO_NO_SIGNAL);
			//}

			if (dk_bgfylz65_config.send_cycle_count != NO_TIMER)
			{
				if (CICycleInt_GetCounter() - dk_bgfylz65_config.send_cycle_count >= (4 * 1000 / CI_CYCLE_MS))
				{
					send_command(dk_bgfylz65_config.TYD,SIO_NO_SIGNAL);
					send_command(dk_bgfylz65_config.SQD,SIO_NO_SIGNAL);
					dk_bgfylz65_config.send_cycle_count = NO_TIMER;
				}
			}
		}
	}	
}

/****************************************************
函数名：   dk_bgcpkz53
功能描述： 宝钢成品库站53#道口
返回值：   void
作者：	   hejh
日期：     2015/08/13
****************************************************/
void dk_bgcpkz53()
{
	CI_BOOL result = CI_TRUE;

	if (dk_bgcpkz53_config.AlarmIndex != NO_INDEX)
	{
		/*获取信号机的状态*/
		if ((dk_bgcpkz53_config.SignalIndex != NO_INDEX)
			&& (gn_signal_state(dk_bgcpkz53_config.SignalIndex) == dk_bgcpkz53_config.SignalState))
		{
			/*获取区段1和区段2的状态*/
			if (dk_bgcpkz53_config.Section1Index != NO_INDEX)
			{
				if (dk_bgcpkz53_config.Section1State == SCS_OCCUPIED)
				{
					if (gn_section_state(dk_bgcpkz53_config.Section1Index) != SCS_CLEARED)
					{
						result = CI_FALSE;
					}
					else
					{
						if (dk_bgcpkz53_config.Section2Index != NO_INDEX)
						{
							if (dk_bgcpkz53_config.Section2State == SCS_OCCUPIED)
							{
								if (gn_section_state(dk_bgcpkz53_config.Section2Index) != SCS_CLEARED)
								{
									result = CI_FALSE;
								}
							}
							else
							{
								if (gn_section_state(dk_bgcpkz53_config.Section2Index) == dk_bgcpkz53_config.Section2State)
								{
									result = CI_FALSE;
								}
							}							
						}
					}
				}
				else
				{
					if (gn_section_state(dk_bgcpkz53_config.Section1Index) == dk_bgcpkz53_config.Section1State)
					{
						result = CI_FALSE;
					}
					else
					{
						if (dk_bgcpkz53_config.Section2Index != NO_INDEX)
						{
							if (dk_bgcpkz53_config.Section2State == SCS_OCCUPIED)
							{
								if (gn_section_state(dk_bgcpkz53_config.Section2Index) != SCS_CLEARED)
								{
									result = CI_FALSE;
								}
							}
							else
							{
								if (gn_section_state(dk_bgcpkz53_config.Section2Index) == dk_bgcpkz53_config.Section2State)
								{
									result = CI_FALSE;
								}
							}							
						}
					}
				}
				
			}
		}
		if (IsTRUE(result))
		{
			send_command(dk_bgcpkz53_config.AlarmIndex,dk_bgcpkz53_config.AlarmState);
		}
		else
		{			
			if (dk_bgcpkz53_config.AlarmState == SIO_HAS_SIGNAL)
			{
				send_command(dk_bgcpkz53_config.AlarmIndex,SIO_NO_SIGNAL);
			}
			else
			{
				send_command(dk_bgcpkz53_config.AlarmIndex,SIO_HAS_SIGNAL);
			}
		}
	}
}

/****************************************************
函数名：   cross_station
功能描述： 跨场作业
返回值：   void
作者：	   hejh
日期：     2015/09/11
****************************************************/
void cross_station()
{
	route_t route_index = NO_INDEX,lead_route = NO_INDEX;
	int16_t i,j,k,m,next_signal = NO_INDEX,section = NO_INDEX;
	static uint8_t route_state = 0;
	CI_BOOL result = CI_FALSE;

	for (i = 0; i < MAX_CROSS_STATION; i++)
	{
		/*if (i == 0)
		{
			CIHmi_SendDebugTips("【Recv1】RouteState:%d",CrossStation1RecviceFromEnd[i].RouteState);
			CIHmi_SendDebugTips("【Recv2】RouteState:%d",CrossStation2RecviceFromEnd[i].RouteState);
		}*/

		/*进路状态*/
		/*站场1*/
		if (CrossStation1SendToStart[i].StartSignal != NO_INDEX)			
		{
			/*信号机及进路状态*/
			if ((gn_belong_route(CrossStation1SendToStart[i].StartSignal) != NO_INDEX)
				&& (CrossStation1SendToStart[i].StartSignal == gr_start_signal(gn_belong_route(CrossStation1SendToStart[i].StartSignal))))
			{
				CrossStation1SendToStart[i].RouteState = gr_state(gn_belong_route(CrossStation1SendToStart[i].StartSignal));
			}
			else if ((gn_belong_route(CrossStation1SendToStart[i].StartSignal) == NO_INDEX)
				&& (RS_FAILURE_TO_BUILD == CrossStation1SendToStart[i].RouteState))
			{
				//CrossStation1SendToStart[i].RouteState = RS_FAILURE_TO_BUILD;
			}
			else
			{
				CrossStation1SendToStart[i].RouteState = RS_ERROR;
			}
		}
		else
		{
			CrossStation1SendToStart[i].RouteState = RS_ERROR;
		}

		/*站场2*/
		if (CrossStation2SendToStart[i].StartSignal != NO_INDEX)
		{
			/*信号机及进路状态*/
			if ((gn_belong_route(CrossStation2SendToStart[i].StartSignal) != NO_INDEX)
				&& (CrossStation2SendToStart[i].StartSignal == gr_start_signal(gn_belong_route(CrossStation2SendToStart[i].StartSignal))))
			{
				CrossStation2SendToStart[i].RouteState = gr_state(gn_belong_route(CrossStation2SendToStart[i].StartSignal));
			}
			else if ((gn_belong_route(CrossStation2SendToStart[i].StartSignal) == NO_INDEX)
				&& (RS_FAILURE_TO_BUILD == CrossStation2SendToStart[i].RouteState))
			{
				//CrossStation2SendToStart[i].RouteState = RS_FAILURE_TO_BUILD;
			}
			else
			{
				CrossStation2SendToStart[i].RouteState = RS_ERROR;
			}
		}
		else
		{
			CrossStation2SendToStart[i].RouteState = RS_ERROR;
		}

		/*调车中途折返*/
		/*站场1*/
		if (CrossStation1SendToEnd[i].StartSignal != NO_INDEX)
		{
			route_index = gn_belong_route(CrossStation1SendToEnd[i].StartSignal);
			if (route_index == NO_INDEX)
			{
				next_signal = CrossStation1SendToEnd[i].StartSignal;
				for (j = 0; j < TOTAL_SIGNAL_NODE; j++)
				{					
					if ((NO_INDEX != next_signal)
						&& IsTRUE(is_signal(next_signal))
						&& (gn_direction(next_signal) == gn_direction(CrossStation1SendToEnd[i].StartSignal)))
					{
						if (NO_INDEX != gn_belong_route(next_signal))
						{
							if ((RS_SIGNAL_OPENED == gr_state(gn_belong_route(next_signal))))
								//&& (SCS_CLEARED != gn_section_state(gr_first_section(gn_belong_route(next_signal)))))
							{
								CrossStation1SendToEnd[i].IsRetun = CI_TRUE;
								break;
							}
							else
							{
								CrossStation1SendToEnd[i].IsRetun = CI_FALSE;
								break;
							}
						}
						else
						{
							CrossStation1SendToEnd[i].IsRetun = CI_FALSE;
							if(next_signal != CrossStation1SendToEnd[i].StartSignal)
							{
								break;
							}
							else
							{
								next_signal = gn_forword(gn_direction(CrossStation1SendToEnd[i].StartSignal),next_signal);
							}
						}
					}
					else
					{
						next_signal = gn_forword(gn_direction(CrossStation1SendToEnd[i].StartSignal),next_signal);
					}
				}
			}
			if ((route_index != NO_INDEX)
				&& (RT_SHUNTING_ROUTE == gr_type(route_index))
				&& (gb_node(gr_end_button(route_index)) == CrossStation1SendToEnd[i].StartSignal)
				&& (gr_other_flag(route_index) == ROF_CROSS_STATION))
			{				
				if ((RS_AUTO_UNLOCK_FAILURE != gr_state(route_index)) || (RS_FAILURE != gr_state(route_index)) 
					|| (RS_CRASH_INTO_SIGNAL != gr_state(route_index)))
				{
					/*并置信号机处折返解锁*/
					if ((gr_state(route_index) == RS_SK_N_SIGNAL_CLOSED)         
						&& (gn_type(gr_start_signal(route_index)) == NT_JUXTAPOSE_SHUNGTING_SIGNAL)
						&& IsTRUE(check_juxtapose_unlock_condition(route_index)))
					{
						CrossStation1SendToEnd[i].IsRetun = CI_TRUE;
					}
					else
					{
						CrossStation1SendToEnd[i].IsRetun = CI_FALSE;
					}
					/*全部未解锁区段解锁*/
					if (IsFALSE(CrossStation1SendToEnd[i].IsRetun)
						&& IsTRUE(is_route_exist(route_index)))
					{
						lead_route = cross_station_check_all_unlock_condition(route_index);
						if (lead_route != NO_INDEX)
						{
							CrossStation1SendToEnd[i].IsRetun = CI_TRUE;
						}
						else
						{
							CrossStation1SendToEnd[i].IsRetun = CI_FALSE;
						}
					}				
					/*部分未解锁区段解锁*/
					if (IsFALSE(CrossStation1SendToEnd[i].IsRetun)
						&& IsTRUE(is_route_exist(route_index))
						&& (gr_state(route_index) == RS_AUTOMATIC_UNLOCKING) 
						&& IsTRUE(cross_station_check_part_unlock_condition(route_index)))
					{
						CrossStation1SendToEnd[i].IsRetun = CI_TRUE;
					}
				}
				else
				{
					CrossStation1SendToEnd[i].IsRetun = CI_FALSE;
				}
			}
		}
		/*站场2*/
		if (CrossStation2SendToEnd[i].StartSignal != NO_INDEX)
		{
			route_index = gn_belong_route(CrossStation2SendToEnd[i].StartSignal);
			if (route_index == NO_INDEX)
			{
				next_signal = CrossStation2SendToEnd[i].StartSignal;
				for (j = 0; j < TOTAL_SIGNAL_NODE; j++)
				{					
					if ((NO_INDEX != next_signal)
						&& IsTRUE(is_signal(next_signal))
						&& (gn_direction(next_signal) == gn_direction(CrossStation2SendToEnd[i].StartSignal)))
					{
						if (NO_INDEX != gn_belong_route(next_signal))
						{
							if ((RS_SIGNAL_OPENED == gr_state(gn_belong_route(next_signal))))
								//&& (SCS_CLEARED != gn_section_state(gr_first_section(gn_belong_route(next_signal)))))
							{
								CrossStation2SendToEnd[i].IsRetun = CI_TRUE;
								break;
							}
							else
							{
								CrossStation2SendToEnd[i].IsRetun = CI_FALSE;
								break;
							}
						}
						else
						{
							CrossStation2SendToEnd[i].IsRetun = CI_FALSE;
							if(next_signal != CrossStation2SendToEnd[i].StartSignal)
							{
								break;
							}
							else
							{
								next_signal = gn_forword(gn_direction(CrossStation2SendToEnd[i].StartSignal),next_signal);
							}
						}
					}
					else
					{
						next_signal = gn_forword(gn_direction(CrossStation2SendToEnd[i].StartSignal),next_signal);
					}
				}
			}
			if ((route_index != NO_INDEX)
				&& (RT_SHUNTING_ROUTE == gr_type(route_index))
				&& (gb_node(gr_end_button(route_index)) == CrossStation2SendToEnd[i].StartSignal)
				&& (gr_other_flag(route_index) == ROF_CROSS_STATION))
			{
				if ((RS_AUTO_UNLOCK_FAILURE != gr_state(route_index)) || (RS_FAILURE != gr_state(route_index)) 
					|| (RS_CRASH_INTO_SIGNAL != gr_state(route_index)))
				{
					/*并置信号机处折返解锁*/
					if ((gr_state(route_index) == RS_SK_N_SIGNAL_CLOSED)         
						&& (gn_type(gr_start_signal(route_index)) == NT_JUXTAPOSE_SHUNGTING_SIGNAL)
						&& IsTRUE(check_juxtapose_unlock_condition(route_index)))
					{
						CrossStation2SendToEnd[i].IsRetun = CI_TRUE;
					}
					else
					{
						CrossStation2SendToEnd[i].IsRetun = CI_FALSE;
					}
					/*全部未解锁区段解锁*/
					if (IsFALSE(CrossStation2SendToEnd[i].IsRetun)
						&& IsTRUE(is_route_exist(route_index)))
					{
						lead_route = cross_station_check_all_unlock_condition(route_index);
						if (lead_route != NO_INDEX)
						{
							CrossStation2SendToEnd[i].IsRetun = CI_TRUE;
						}
						else
						{
							CrossStation2SendToEnd[i].IsRetun = CI_FALSE;
						}
					}				
					/*部分未解锁区段解锁*/
					if (IsFALSE(CrossStation2SendToEnd[i].IsRetun)
						&& IsTRUE(is_route_exist(route_index))
						&& (gr_state(route_index) == RS_AUTOMATIC_UNLOCKING) 
						&& IsTRUE(cross_station_check_part_unlock_condition(route_index)))
					{
						CrossStation2SendToEnd[i].IsRetun = CI_TRUE;
					}
				}
				else
				{
					CrossStation2SendToEnd[i].IsRetun = CI_FALSE;
				}
			}
		}

		/*锁闭区段*/
		for (j = 0; j < TOTAL_SIGNAL_NODE; j++)
		{
			if (strcmp_no_case(CrossStation1RecviceFromStart[i].LinkSignal,gn_name(j)) == 0)
			{
				/*发车*/
				if (IsTRUE(CrossStation1RecviceFromStart[i].RouteDirection))
				{
					CrossStation1SendToEnd[i].RouteDirection = CI_FALSE;
					result = CI_FALSE;
					if ((CrossStation1SendToEnd[i].StartSignal != NO_INDEX)
						&& (gn_belong_route(CrossStation1SendToEnd[i].StartSignal) != NO_INDEX)
						&& (gb_node(gr_end_button(gn_belong_route(CrossStation1SendToEnd[i].StartSignal))) == CrossStation1SendToEnd[i].StartSignal)
						&& (gr_other_flag(gn_belong_route(CrossStation1SendToEnd[i].StartSignal)) == ROF_CROSS_STATION))
					{
						result = CI_TRUE;
						CrossStation1SendToEnd[i].RouteDirection = CI_TRUE;
					}
					if (IsFALSE(result)
						&& (CrossStation1SendToEnd[i].StartSignal != NO_INDEX)
						&& (gn_belong_route(CrossStation1SendToEnd[i].StartSignal) != NO_INDEX)
						&& (gb_node(gr_end_button(gn_belong_route(CrossStation1SendToEnd[i].StartSignal))) == CrossStation1SendToEnd[i].StartSignal)
						&& (gr_other_flag(gn_belong_route(CrossStation1SendToEnd[i].StartSignal)) == ROF_ERROR))
					{
						result = CI_TRUE;
					}
					if (IsFALSE(result))
					{
						for (m = 0; m < MAX_CROSS_STATION; m++)
						{
							if (IsTRUE(CrossStationRoute[m].IsExist))
							{
								result = CI_TRUE;
								CrossStation1SendToEnd[i].RouteDirection = CI_TRUE;
								break;
							}
						}
					}
					if (IsFALSE(result))
					{
						section = j;
						for (k = 0; k < TOTAL_SIGNAL_NODE; k++)
						{
							if (IsTRUE(is_section(section)))
							{
								if (IsTRUE(CrossStation1RecviceFromStart[i].LockSection))
								{
									sn_locked_state(section,LT_LOCKED);
								}
								else
								{
									cn_locked_state(section,LT_LOCKED);
								}
								break;
							}
							else
							{
								section = gn_backword(gn_direction(j),section);
							}
						}
					}
				}	
			}
			if (strcmp_no_case(CrossStation2RecviceFromStart[i].LinkSignal,gn_name(j)) == 0)
			{
				/*发车*/
				if (IsTRUE(CrossStation2RecviceFromStart[i].RouteDirection))
				{
					CrossStation2SendToEnd[i].RouteDirection = CI_FALSE;
					result = CI_FALSE;
					if ((CrossStation2SendToEnd[i].StartSignal != NO_INDEX)
						&& (gn_belong_route(CrossStation2SendToEnd[i].StartSignal) != NO_INDEX)
						&& (gb_node(gr_end_button(gn_belong_route(CrossStation2SendToEnd[i].StartSignal))) == CrossStation2SendToEnd[i].StartSignal)
						&& (gr_other_flag(gn_belong_route(CrossStation2SendToEnd[i].StartSignal)) == ROF_CROSS_STATION))
					{
						result = CI_TRUE;
						CrossStation2SendToEnd[i].RouteDirection = CI_TRUE;
					}
					if (IsFALSE(result)
						&& (CrossStation2SendToEnd[i].StartSignal != NO_INDEX)
						&& (gn_belong_route(CrossStation2SendToEnd[i].StartSignal) != NO_INDEX)
						&& (gb_node(gr_end_button(gn_belong_route(CrossStation2SendToEnd[i].StartSignal))) == CrossStation2SendToEnd[i].StartSignal)
						&& (gr_other_flag(gn_belong_route(CrossStation2SendToEnd[i].StartSignal)) == ROF_ERROR))
					{
						result = CI_TRUE;
					}
					if (IsFALSE(result))
					{
						for (m = 0; m < MAX_CROSS_STATION; m++)
						{
							if (IsTRUE(CrossStationRoute[m].IsExist))
							{
								result = CI_TRUE;
								CrossStation2SendToEnd[i].RouteDirection = CI_TRUE;
								break;
							}
						}
					}
					if (IsFALSE(result))
					{
						section = j;
						for (k = 0; k < TOTAL_SIGNAL_NODE; k++)
						{
							if (IsTRUE(is_section(section)))
							{
								if (IsTRUE(CrossStation2RecviceFromStart[i].LockSection))
								{
									sn_locked_state(section,LT_LOCKED);
								}
								else
								{
									cn_locked_state(section,LT_LOCKED);
								}
								break;
							}
							else
							{
								section = gn_backword(gn_direction(j),section);
							}
						}
					}
				}
			}
		}		
	}
}

/****************************************************
函数名：   cross_station_function
功能描述： 跨场作业虚拟进路功能操作
返回值：   CI_BOOL
参数：     CI_BOOL delay_unlock_flag
作者：	   hejh
日期：     2015/10/21
****************************************************/
CI_BOOL cross_station_function(CI_BOOL delay_unlock_flag)
{
	int16_t i,j,k,approch_section = NO_INDEX;
	CI_BOOL result = CI_FALSE;

	/*hjh 2015-10-21 针对跨场作业虚拟进路处理*/
	for (i = 0; i < MAX_CROSS_STATION; i++)
	{
		if (CrossStationRoute[i].RouteSignal != NO_INDEX)
		{
			/*延时解锁*/
			if (IsTRUE(delay_unlock_flag))
			{
				if (IsTRUE(is_node_timer_run(CrossStationRoute[i].RouteSignal)))
				{
					if (IsTRUE(is_node_complete_timer(CrossStationRoute[i].RouteSignal)))
					{
						cn_locked_state(CrossStationRoute[i].RouteSection,LT_LOCKED);
						CrossStationRoute[i].IsExist = CI_FALSE;
						/*解锁跨场作业区段*/
						for (k = 0; k < MAX_CROSS_STATION; k++)
						{
							if ((CrossStation1SendToEnd[k].StartSignal != NO_INDEX)
								&& (CrossStation1SendToEnd[k].StartSignal == CrossStationRoute[i].StartSignal))
							{
								CrossStation1SendToEnd[k].LockSection = CI_FALSE;
							}
							if ((CrossStation2SendToEnd[k].StartSignal != NO_INDEX)
								&& (CrossStation2SendToEnd[k].StartSignal == CrossStationRoute[i].StartSignal))
							{
								CrossStation2SendToEnd[k].LockSection = CI_FALSE;
							}
						}
					}
				}
			}
			/*功能操作*/
			else
			{
				if (CrossStationRoute[i].RouteSignal == gb_node(second_button))
				{
					result = CI_TRUE;
					switch(first_button)
					{
						/*取消进路*/	
					case FB_CANCEL_ROUTE : 
						if (IsTRUE(is_node_locked(CrossStationRoute[i].RouteSection,LT_LOCKED)))
						{
							approch_section = CrossStationRoute[i].RouteSignal;
							for (j = 0; j < TOTAL_SIGNAL_NODE; j++)
							{
								if (IsTRUE(is_section(approch_section)))
								{
									if (SCS_CLEARED == gn_section_state(approch_section))
									{
										send_signal_command(CrossStationRoute[i].RouteSignal,SGS_A);
										cn_locked_state(CrossStationRoute[i].RouteSection,LT_LOCKED);
										CrossStationRoute[i].IsExist = CI_FALSE;
										/*解锁跨场作业区段*/
										for (k = 0; k < MAX_CROSS_STATION; k++)
										{
											if ((CrossStation1SendToEnd[k].StartSignal != NO_INDEX)
												&& (CrossStation1SendToEnd[k].StartSignal == CrossStationRoute[i].StartSignal))
											{
												CrossStation1SendToEnd[k].LockSection = CI_FALSE;
											}
											if ((CrossStation2SendToEnd[k].StartSignal != NO_INDEX)
												&& (CrossStation2SendToEnd[k].StartSignal == CrossStationRoute[i].StartSignal))
											{
												CrossStation2SendToEnd[k].LockSection = CI_FALSE;
											}
										}

									}
									else
									{
										CIHmi_SendNormalTips("接近区段占用：%s",gn_name(CrossStationRoute[i].RouteSignal));
									}
									break;
								}
								else
								{
									approch_section = gn_backword(gn_direction(CrossStationRoute[i].RouteSignal),approch_section);
								}
							}						
						}
						else
						{
							CIHmi_SendNormalTips("错误办理：%s",gb_node(second_button) == NO_INDEX ? gn_name(second_button) : gn_name(gb_node(second_button)));
						}
						break;
						/*人工解锁*/	
					case FB_HUMAN_UNLOCK :
						if (IsTRUE(is_node_locked(CrossStationRoute[i].RouteSection,LT_LOCKED)))
						{
							approch_section = CrossStationRoute[i].RouteSignal;
							for (j = 0; j < TOTAL_SIGNAL_NODE; j++)
							{
								if (IsTRUE(is_section(approch_section)))
								{
									if (SCS_CLEARED == gn_section_state(approch_section))
									{
										send_signal_command(CrossStationRoute[i].RouteSignal,SGS_A);
										cn_locked_state(CrossStationRoute[i].RouteSection,LT_LOCKED);
										CrossStationRoute[i].IsExist = CI_FALSE;
										/*解锁跨场作业区段*/
										for (k = 0; k < MAX_CROSS_STATION; k++)
										{
											if ((CrossStation1SendToEnd[k].StartSignal != NO_INDEX)
												&& (CrossStation1SendToEnd[k].StartSignal == CrossStationRoute[i].StartSignal))
											{
												CrossStation1SendToEnd[k].LockSection = CI_FALSE;
											}
											if ((CrossStation2SendToEnd[k].StartSignal != NO_INDEX)
												&& (CrossStation2SendToEnd[k].StartSignal == CrossStationRoute[i].StartSignal))
											{
												CrossStation2SendToEnd[k].LockSection = CI_FALSE;
											}
										}
									}
									else
									{
										send_signal_command(CrossStationRoute[i].RouteSignal,SGS_A);
										if (IsTRUE(is_node_timer_run(CrossStationRoute[i].RouteSignal)))
										{
											CIHmi_SendNormalTips("正在延时解锁：%s",gn_name(CrossStationRoute[i].RouteSignal));
										}
										else
										{
											CIHmi_SendNormalTips("接近区段占用，延时解锁");
											sn_start_timer(CrossStationRoute[i].RouteSignal,SECONDS_30,DTT_UNLOCK);
										}									
									}
									break;
								}
								else
								{
									approch_section = gn_backword(gn_direction(CrossStationRoute[i].RouteSignal),approch_section);
								}
							}
						}
						else
						{
							CIHmi_SendNormalTips("错误办理：%s",gb_node(second_button) == NO_INDEX ? gn_name(second_button) : gn_name(gb_node(second_button)));
						}
						break;
						/*区故解*/
					case FB_SECTION_UNLOCK : 
						if (IsTRUE(is_node_locked(CrossStationRoute[i].RouteSection,LT_LOCKED)))
						{
							if (gn_signal_state(CrossStationRoute[i].RouteSignal) == SGS_B)
							{
								send_signal_command(CrossStationRoute[i].RouteSignal,SGS_A);
							}
							else
							{
								approch_section = CrossStationRoute[i].RouteSignal;
								for (j = 0; j < TOTAL_SIGNAL_NODE; j++)
								{
									if (IsTRUE(is_section(approch_section)))
									{
										if (SCS_CLEARED == gn_section_state(approch_section))
										{
											cn_locked_state(CrossStationRoute[i].RouteSection,LT_LOCKED);
											CrossStationRoute[i].IsExist = CI_FALSE;
											/*解锁跨场作业区段*/
											for (k = 0; k < MAX_CROSS_STATION; k++)
											{
												if ((CrossStation1SendToEnd[k].StartSignal != NO_INDEX)
													&& (CrossStation1SendToEnd[k].StartSignal == CrossStationRoute[i].StartSignal))
												{
													CrossStation1SendToEnd[k].LockSection = CI_FALSE;
												}
												if ((CrossStation2SendToEnd[k].StartSignal != NO_INDEX)
													&& (CrossStation2SendToEnd[k].StartSignal == CrossStationRoute[i].StartSignal))
												{
													CrossStation2SendToEnd[k].LockSection = CI_FALSE;
												}
											}
										}
										else
										{
											if (IsTRUE(is_node_timer_run(CrossStationRoute[i].RouteSignal)))
											{
												CIHmi_SendNormalTips("正在延时解锁：%s",gn_name(CrossStationRoute[i].RouteSignal));
											}
											else
											{
												CIHmi_SendNormalTips("接近区段占用，延时解锁");
												sn_start_timer(CrossStationRoute[i].RouteSignal,SECONDS_30,DTT_UNLOCK);
											}									
										}
										break;
									}
									else
									{
										approch_section = gn_backword(gn_direction(CrossStationRoute[i].RouteSignal),approch_section);
									}
								}
							}
						}
						else
						{
							CIHmi_SendNormalTips("错误办理：%s",gb_node(second_button) == NO_INDEX ? gn_name(second_button) : gn_name(gb_node(second_button)));
						}
						break;
						/*关闭信号*/	
					case FB_CLOSE_SIGNAL : 
						if (gn_signal_state(CrossStationRoute[i].RouteSignal) == SGS_B)
						{
							send_signal_command(CrossStationRoute[i].RouteSignal,SGS_A);
						}
						else
						{
							CIHmi_SendNormalTips("错误办理：%s",gb_node(second_button) == NO_INDEX ? gn_name(second_button) : gn_name(gb_node(second_button)));
						}
						break;
						/*重复开放*/	
					case FB_REOPEN_SIGNAL :  
						if (gn_signal_state(CrossStationRoute[i].RouteSignal) == SGS_A)
						{
							send_signal_command(CrossStationRoute[i].RouteSignal,SGS_B);
						}
						else
						{
							CIHmi_SendNormalTips("错误办理：%s",gb_node(second_button) == NO_INDEX ? gn_name(second_button) : gn_name(gb_node(second_button)));
						}
						break;
					}
					break;
				}
			}
		}
	}
	return result;
}

/****************************************************
函数名：   cross_station_route
功能描述： 跨场作业虚拟进路关闭信号和区段解锁
返回值：   void
作者：	   hejh
日期：     2015/10/22
****************************************************/
void cross_station_route()
{
	int16_t i,j,k,approch_section = NO_INDEX;
	static uint8_t cross_station_route_state = 0;

	for (i = 0; i < MAX_CROSS_STATION; i++)
	{
		if ((CrossStationRoute[i].RouteSignal != NO_INDEX)
			&& (CrossStationRoute[i].RouteSection != NO_INDEX))
		{
			/*信号开放前*/
			if ((gn_signal_state(CrossStationRoute[i].RouteSignal) != SGS_B)
				&& (cross_station_route_state == 0))
			{
				if(IsTRUE(CrossStationRoute[i].IsExist)
					&& (cross_station_route_state == 0))
				{
					if (IsTRUE(is_node_locked(CrossStationRoute[i].RouteSection,LT_LOCKED)))
					{
						send_signal_command(CrossStationRoute[i].RouteSignal,SGS_B);					
					}
					else
					{
						sn_locked_state(CrossStationRoute[i].RouteSection,LT_LOCKED);
						/*锁闭跨场作业区段*/
						for (j = 0; j < MAX_CROSS_STATION; j++)
						{
							if ((CrossStation1SendToEnd[j].StartSignal != NO_INDEX)
								&& (CrossStation1SendToEnd[j].StartSignal == CrossStationRoute[i].StartSignal))
							{
								CrossStation1SendToEnd[j].LockSection = CI_TRUE;
								CrossStation1SendToEnd[i].RouteDirection = CI_TRUE;
							}
							if ((CrossStation2SendToEnd[j].StartSignal != NO_INDEX)
								&& (CrossStation2SendToEnd[j].StartSignal == CrossStationRoute[i].StartSignal))
							{
								CrossStation2SendToEnd[j].LockSection = CI_TRUE;
								CrossStation2SendToEnd[i].RouteDirection = CI_TRUE;
							}
						}
					}
				}				
			}
			/*信号开放后*/
			else
			{
				if (IsTRUE(is_node_locked(CrossStationRoute[i].RouteSection,LT_LOCKED))
					&& (gn_signal_state(CrossStationRoute[i].RouteSignal) == SGS_B))
				{
					approch_section = CrossStationRoute[i].RouteSignal;
					for (j = 0; j < TOTAL_SIGNAL_NODE; j++)
					{
						if (IsTRUE(is_section(approch_section)))
						{
							/*虚拟进路状态机*/
							switch(cross_station_route_state)
							{
								/*初始化*/
								case 0:
									if (SCS_CLEARED != gn_section_state(approch_section))
									{
										cross_station_route_state = 1;
									}
									break;
									/*接近区段占用*/
								case 1:
									if (SCS_CLEARED != gn_section_state(CrossStationRoute[i].RouteSection))
									{
										cross_station_route_state = 2;
									}
									break;							
								case 2:
									/*进路内区段出清,折返*/
									if (SCS_CLEARED == gn_section_state(CrossStationRoute[i].RouteSection))
									{
										cross_station_route_state = 3;
										send_signal_command(CrossStationRoute[i].RouteSignal,SGS_A);
									}
									/*接近区段出清*/
									if (SCS_CLEARED == gn_section_state(approch_section))
									{
										cross_station_route_state = 4;
										send_signal_command(CrossStationRoute[i].RouteSignal,SGS_A);
									}
									break;
							}
							break;
						}
						else
						{
							approch_section = gn_backword(gn_direction(CrossStationRoute[i].RouteSignal),approch_section);
						}
					}						
				}
				if (IsTRUE(is_node_locked(CrossStationRoute[i].RouteSection,LT_LOCKED))
					&& (gn_signal_state(CrossStationRoute[i].RouteSignal) != SGS_B))
				{
					approch_section = CrossStationRoute[i].RouteSignal;
					for (j = 0; j < TOTAL_SIGNAL_NODE; j++)
					{
						if (IsTRUE(is_section(approch_section)))
						{
							if (cross_station_route_state == 3)
							{
								/*接近区段出清*/
								if (SCS_CLEARED == gn_section_state(approch_section))
								{
									cross_station_route_state = 0;
									cn_locked_state(CrossStationRoute[i].RouteSection,LT_LOCKED);
									CrossStationRoute[i].IsExist = CI_FALSE;
									/*解锁跨场作业区段*/
									for (k = 0; k < MAX_CROSS_STATION; k++)
									{
										if ((CrossStation1SendToEnd[k].StartSignal != NO_INDEX)
											&& (CrossStation1SendToEnd[k].StartSignal == CrossStationRoute[i].StartSignal))
										{
											CrossStation1SendToEnd[k].LockSection = CI_FALSE;
										}
										if ((CrossStation2SendToEnd[k].StartSignal != NO_INDEX)
											&& (CrossStation2SendToEnd[k].StartSignal == CrossStationRoute[i].StartSignal))
										{
											CrossStation2SendToEnd[k].LockSection = CI_FALSE;
										}
									}
								}
							}
							if (cross_station_route_state == 4)
							{
								/*区段出清*/
								if (SCS_CLEARED == gn_section_state(CrossStationRoute[i].RouteSection))
								{
									cross_station_route_state = 0;
									cn_locked_state(CrossStationRoute[i].RouteSection,LT_LOCKED);
									CrossStationRoute[i].IsExist = CI_FALSE;
									/*解锁跨场作业区段*/
									for (k = 0; k < MAX_CROSS_STATION; k++)
									{
										if ((CrossStation1SendToEnd[k].StartSignal != NO_INDEX)
											&& (CrossStation1SendToEnd[k].StartSignal == CrossStationRoute[i].StartSignal))
										{
											CrossStation1SendToEnd[k].LockSection = CI_FALSE;
										}
										if ((CrossStation2SendToEnd[k].StartSignal != NO_INDEX)
											&& (CrossStation2SendToEnd[k].StartSignal == CrossStationRoute[i].StartSignal))
										{
											CrossStation2SendToEnd[k].LockSection = CI_FALSE;
										}
									}
								}
							}
							break;
						}
						else
						{
							approch_section = gn_backword(gn_direction(CrossStationRoute[i].RouteSignal),approch_section);
						}
					}						
				}
			}			
		}
	}
}

/****************************************************
函数名：   cross_station_check_all_unlock_condition
功能描述： 跨场折返检查全部未解锁进路条件
返回值：   route_t
参数：     route_t route_index
作者：	   hejh
日期：     2015/10/20
****************************************************/
route_t cross_station_check_all_unlock_condition(route_t route_index)
{
	CI_BOOL result = CI_TRUE;
	route_t return_route = NO_INDEX;
	int16_t return_signal = NO_INDEX;
	
	/*参数检查*/
	if (IsFALSE(is_route_exist(route_index)))
	{
		process_warning(ERR_INDEX,"");
		result = CI_FALSE;
	}
	/*获取折返信号*/
	return_signal = cross_station_search_return_signal(route_index,CI_TRUE);	
	/*折返信号未关闭*/
	if ((return_signal != NO_INDEX) && (SGS_B == gn_signal_state(return_signal)))
	{
		return_route = gn_belong_route(return_signal);;
	}
	else
	{
		return_route = NO_INDEX;
	}	

	if (return_route == NO_INDEX)
	{
		result = CI_FALSE;
	}
	else
	{
		/*车列进入折返进路*/
		if ((return_signal == gr_start_signal(return_route))
			&& (RS_SIGNAL_OPENED == gr_state(return_route)))
			//&& (SCS_CLEARED != gn_section_state(gr_first_section(return_route))))
		{
			/*确认道岔位置正确*/
			if (IsFALSE(check_switch_location(return_route)))
			{
				result = CI_FALSE;
			}
		}
		else
		{				
			result = CI_FALSE;
		}
	}	

	if (IsFALSE(result))
	{
		return_route = NO_INDEX;
	}
	return return_route;
}

/****************************************************
函数名：   cross_station_check_part_unlock_condition
功能描述： 跨场折返检查部分未解锁进路条件
返回值：   CI_BOOL
参数：     route_t route_index
作者：	   hejh
日期：     2015/10/15
****************************************************/
CI_BOOL cross_station_check_part_unlock_condition(route_t route_index)
{
	CI_BOOL result = CI_TRUE;
	node_t return_signal = NO_INDEX;
	route_t return_route = NO_INDEX;
	int16_t i,index,node;
	int16_t node_count = gr_nodes_count(route_index);

	/*参数检查*/
	if (IsFALSE(is_route_exist(route_index)))
	{
		process_warning(ERR_INDEX,"");
		result = CI_FALSE;
	}
	/*获取折返信号*/
	return_signal = cross_station_search_return_signal(route_index,CI_FALSE);	
	/*折返信号已关闭*/
	if (return_signal != NO_INDEX)
	{
		return_route = gn_belong_route(return_signal);;
	}
	else
	{
		return_route = NO_INDEX;
	}	
	
	if (return_route == NO_INDEX)
	{
		result = CI_FALSE;
	}
	else
	{
		/*车列进入折返进路*/
		if (!((return_signal == gr_start_signal(return_route))
			//&& (RS_SK_N_SIGNAL_CLOSED == gr_state(return_route))
			&& (SCS_CLEARED != gn_section_state(gr_first_section(return_route)))))
		{
			result = CI_FALSE;
		}		
		/*区段空闲检查*/
		index = gr_node_index_route(route_index,return_signal);		
		for(i = node_count - 1 ;  i > index ; i-- )
		{
			node = gr_node(route_index,i);
			/*道岔位置检查*/
			if (IsTRUE(is_section(node))) 
			{
				if (NO_NODE != cross_station_is_switch_location_right(return_route,node,i))
				{
					result = CI_FALSE;
				}				
			}
		}		
	}
	return result;
}

/****************************************************
函数名：   cross_station_search_return_signal
功能描述： 搜索跨场折返信号机
返回值：   int16_t
参数：     route_t route_index
参数：     CI_BOOL is_all_unlock_route
作者：	   hejh
日期：     2015/10/20
****************************************************/
int16_t cross_station_search_return_signal(route_t route_index,CI_BOOL is_all_unlock_route)
{
	int16_t i,j,node = NO_INDEX,start_signal = NO_INDEX,return_signal = NO_INDEX;
	route_t lead_route = NO_INDEX,temp_route = NO_INDEX;

	/*参数检查*/
	if(IsFALSE(is_route_exist(route_index)))
	{
		return_signal = NO_NODE;
	}
	/*确认进路类型*/
	if (RT_SHUNTING_ROUTE == gr_type(route_index))
	{	
		/*全部未解锁进路*/
		if (IsTRUE(is_all_unlock_route))
		{
			temp_route = route_index;
			/*hjh 2013-4-24 多条牵出进路时的折返解锁*/
			for (j = 0; j < MAX_ROUTE; j++)
			{
				/*向后寻找折返进路*/
				lead_route = gr_backward(temp_route);
				if ((lead_route != NO_INDEX) && (RT_SHUNTING_ROUTE == gr_type(lead_route)))
				{
					temp_route = lead_route;
					j = 0;
				}
				else
				{
					lead_route = temp_route;
					break;
				}

			}
			if (lead_route != NO_INDEX)
			{
				start_signal = gr_start_signal(lead_route);
				/*存在部分未解锁进路*/
				if (gn_belong_route(start_signal) != lead_route)
				{
					/*折返信号机在本进路上*/
					for ( i = gr_nodes_count(lead_route) - 1; i > 0 ; i--)
					{
						node = gr_node(lead_route,i);
						/*折返信号机在本进路上*/
						if (IsTRUE(is_signal(node)) 
							&& (gn_belong_route(node) != NO_ROUTE)
							&& (gn_belong_route(node) != lead_route)
							&& (gr_state(gn_belong_route(node)) == RS_SIGNAL_OPENED))
						{
							/*折返信号机的方向和本进路的方向必须是相反的*/
							if (((gn_direction(node) == DIR_DOWN) && (gr_direction(lead_route) == DIR_UP))
								|| ((gn_direction(node) == DIR_UP) && (gr_direction(lead_route) == DIR_DOWN)))
							{
								return_signal = node;
								break;
							}

						}
					}
				}
				/*不存在部分未解锁进路*/
				else
				{
					if ((NT_DIFF_SHUNTING_SIGNAL == gn_type(start_signal)) 
						|| (NT_JUXTAPOSE_SHUNGTING_SIGNAL == gn_type(start_signal))
						|| (NT_ROUTE_SIGNAL == gn_type(start_signal)))
					{
						/*折返信号机不在本进路上*/
						node = gn_another_signal(start_signal);
						/*获取到的节点不是信号机*/
						if ((node != NO_NODE) && IsFALSE(is_signal(node)))
						{
							if (gr_direction(lead_route) == DIR_UP)
							{
								node = gn_previous(node);
							}
							else
							{
								node = gn_next(node);
							}
						}

						if ((node != NO_INDEX)
							&& (gn_belong_route(node) != NO_ROUTE)
							&& (gr_state(gn_belong_route(node) == RS_SIGNAL_OPENED)))
						{
							/*折返信号机的方向和本进路的方向必须是相反的*/
							if (((gn_direction(node) == DIR_DOWN) && (gr_direction(route_index) == DIR_UP))
								|| ((gn_direction(node) == DIR_UP) && (gr_direction(route_index) == DIR_DOWN)))
							{
								return_signal = node;
							}
						}
					}
				}
			}
		}
		/*部分未解锁进路*/
		else
		{
			for ( i = gr_nodes_count(route_index) - 1; i>0 ; i--)
			{
				node = gr_node(route_index,i);
				/*折返信号机在本进路上，折返信号机已经正常关闭*/
				if (IsTRUE(is_signal(node)) 
					&& (gn_belong_route(node) != NO_ROUTE)
					&& (gn_belong_route(node) != route_index))
				{
					/*折返信号机的方向和本进路的方向必须是相反的*/
					if (((gn_direction(node) == DIR_DOWN) && (gr_direction(route_index) == DIR_UP))
						|| ((gn_direction(node) == DIR_UP) && (gr_direction(route_index) == DIR_DOWN)))
					{
						return_signal = node;
						break;
					}

				}
			}
		}		
	}
	return return_signal;
}

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
node_t cross_station_is_switch_location_right(route_t current_route,node_t current_node,index_t current_ordinal)
{
	node_t result = NO_NODE;
	index_t i,j,k;
	node_t node_count = gr_nodes_count(current_route);
	node_t switch_section,current_switch;

	/*参数检查*/
	if((gn_belong_route(current_node) == current_route) && 
		IsTRUE(is_node_locked(current_node, LT_LOCKED)))
	{
		/*道岔未经检查需要从当前的信号点开始往后检查，即对未解锁的进路上的道岔位置的检查*/
		for (i = current_ordinal; i < node_count; i++)
		{
			switch_section = gr_node(current_route,i);	
			/*首先要确定道岔是在本进路上，避免数据错误导致错误的解锁*/
			if ((gn_type(switch_section) == NT_SWITCH_SECTION) && 
				(gn_belong_route(switch_section) == current_route))
			{				
				/*对道岔区段的多个道岔逐个判断*/
				for (j = 0; j < MAX_SWITCH_PER_SECTION; j ++)
				{
					current_switch = gn_section_switch(switch_section,j);
					/*返回值检查*/
					if (current_switch != NO_INDEX)
					{
						for (k = 0 ; k < gr_switches_count(current_route); k++ )
						{	
							/*必须是本进路上的道岔*/
							/*hjh 2014-7-16 判断不是带动道岔*/
							if ( (gr_switch(current_route,k) == current_switch)
								&& (gn_switch_state(current_switch) != gr_switch_location(current_route,k))
								&& (IsFALSE(is_follow_switch(current_route,k))))
							{
								result = current_switch;
								CIHmi_SendNormalTips("道岔位置错误：%s",gn_name(current_switch));
							}
						}
					}
				}
			}
		}
	}
	return result;
}

/****************************************************
函数名：   cross_station_init
功能描述： 跨场作业进路状态初始化
返回值：   void
作者：	   hejh
日期：     2015/11/03
****************************************************/
void cross_station_init()
{
	int16_t i;
	for (i = 0; i < MAX_CROSS_STATION; i++)
	{
		CrossStation1SendToStart[i].RouteState = RS_ERROR;
		CrossStation2SendToStart[i].RouteState = RS_ERROR;
	}
}