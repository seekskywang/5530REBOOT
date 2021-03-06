/******************** (C) COPYRIGHT 2014 KUNKIN********************
 * 文件名  ：my_register
 * 作者    ：李振
 * 描述    ：库函数版
 * 内容    ：Define register定义系统使用的寄存器，全局变量
 * 注意		 ：头文件中只能有声明，不能有定义
 * 修改日期：2014-12-9
*********************************************************************/

#include "STM32F4xx.h"
#ifndef __my_register_h__
#define __my_register_h__
//=============================================================================

//=============================================================================
#define NOP   __nop();	  //空指令

//============================================================================
extern struct bitDefine flagA,flagB,flagC,flagD,flagE,flagF,flagG;
#define   Flag_ADC_Full             (flagA.bit1)//DMA传输完成标志位 
#define   Flag_BEEP						    	(flagA.bit2)//蜂鸣器触发标志
#define   Flag_Key_Scan							(flagA.bit3)//按键定时扫描标志
#define   UART_SEND_flag						(flagA.bit4)//串口发送完成标志
#define   UART_Buffer_Rece_flag			(flagA.bit5)//串口接收完成标志
#define   Flag_Swtich_ON      			(flagA.bit6)//启动按键处于ON标志
#define   Flag_0_5S      		      	(flagA.bit7)//显示设置电压闪烁标志

#define   Flag_1S      		         	(flagB.bit0)//显示设置电流闪烁标志
#define   Flag_5S      		        	(flagB.bit1)//退出设置状态标志
#define   Flag_Long                 (flagB.bit2)//按键长按标志
#define   Flag_DAC_OFF              (flagB.bit3)//鏄惁杞崲DAC鎺у埗鍊兼爣蹇�
#define   Flag_SetA                 (flagB.bit5)//进入设置电流标志位
#define   Flag_SetV                 (flagB.bit6)//进入设置电压标志位 
#define   Flag_DisSet               (flagB.bit7)//显示设置电压和电流标志位

#define   Flag_GE                   (flagC.bit0)//个位闪烁标志
#define   Flag_SHI                  (flagC.bit1)//十位闪烁标志
#define   Flag_BAI                  (flagC.bit2)//百位闪烁标志
#define   Flag_QIAN                 (flagC.bit3)//千位闪烁标志
#define   Flag_Beep_OFF             (flagC.bit6)//蜂鸣器关闭标志

#define   flag_Tim_USART            (flagD.bit0)//串口计算清零标志
#define   flag_Test                 (flagD.bit1)//自动测试起停标志
#define   flag_Test_PASS            (flagD.bit2)//自动测试PASS标志
#define   flag_Test_FAIL            (flagD.bit3)//自动测试FAIL标志
#define   flag_Test_Time            (flagD.bit4)//自动测试等待时间标志
#define   flag_Test_Start           (flagD.bit5)//自动测试比较开始标志
#define   flag_Test_Min             (flagD.bit6)//自动测试下限标志
#define   flag_Test_MAX             (flagD.bit7)//自动测试上限标志

#define   flag_Test_Door             (flagE.bit0)//自动测试门槛标志
#define   flag_Test_SetTime          (flagE.bit1)//自动测试时间标志 
#define   flag_OVP                   (flagE.bit2)//OVP标志
#define   flag_OCP                   (flagE.bit3)//OCP标志
#define   flag_Doub                  (flagE.bit4)//按键双击
#define   flag_t_Doub                (flagE.bit5)//按键双击计时标志
#define   flag_Time_SW               (flagE.bit6)//定时功能开启标志
#define   flag_CC_MODE               (flagE.bit7)//工作模式标志

#define   flag_Adjust                (flagF.bit0)//鏍″噯妯″紡鏍囧織浣�
#define   flag_ADJ_ON                (flagF.bit1)//
#define   flag_ADJ_VL                (flagF.bit2)//
#define   flag_ADJ_VH                (flagF.bit3)//
#define   flag_ADJ_ALCC              (flagF.bit4)//
#define   flag_ADJ_AHCC              (flagF.bit5)//
#define   flag_ADJ_ALCV              (flagF.bit6)//
#define   flag_ADJ_AHCV              (flagF.bit7)//

#define   flag_DisVL                 (flagG.bit0)//
#define   flag_OverV                 (flagG.bit1)//
#define   flag_OverACC               (flagG.bit2)//
#define   flag_OverACV               (flagG.bit3)//
#define   flag_Just_Usart            (flagG.bit4)//
#define   flag_FAN_ON                (flagG.bit5)//
#define   flag_ADJ_FAN               (flagG.bit6)//
//=============================================================================
extern vu32 GE_V;
extern vu32 SHI_V;
extern vu32 BAI_V;
extern vu32 QIAN_V;  //数码管扫描段码
extern vu32 GE_A;
extern vu32 SHI_A;
extern vu32 BAI_A;
extern vu32 QIAN_A;
//--------------------------
extern vu8 NewKey;
extern vu8 DAC_Flag;
extern vu8 Beep_Flag;
extern vu8 t_beep;
extern vu8 t_KeyScan;
extern vu8 t_LEDScan;  //定时计数器
extern vu8 t_KEYON;
extern vu16 t_0_5S;
extern vu16 t_1S;
extern vu16 t_5S;
extern vu16 t_Basket;
extern vu16 Basket_V;
extern vu16 Basket_A;
extern vu16 t_OFF_V;
extern vu16 t_ON_V;
extern vu16 t_OFF_A;
extern vu16 t_ON_A;
extern vu8 t_USART;
extern vu16 ADJ_Write;//校准时需写入的实际值
extern vu8 OFFSET_Zore;//零点值
extern vu8 ADDR;
extern vu8 t_lock;
extern vu8 t_onoff;
extern vu16 MAX_limit;
extern vu16 MIN_limit;
extern vu16 Test_Time;
extern vu16 Test_Time1;
extern vu16 Time_sw;
extern vu16 Test_Daley;
extern vu16 Test_C_Door;
extern vu16 TEMP_OFFSET;//温度补偿值
//--------------------------
extern vu8 UART_Buffer_Size;//串口接收数据长度
extern vu8 Transmit_BUFFERsize;
//---------------------------
extern vu16 NTC_value;   //AD值
extern vu16 Imon_value;
extern vu16 Imon1_value;
extern vu16 Vmon_value;
extern vu16 Vmon1_value;
extern vu16 Rmon_value;

extern float DISS_Voltage;
extern  float DISS_POW_Voltage;
extern float DISS_Current;
extern float DISS_POW_Current;
extern float DISS_R;

extern vu16 Run_Control[14];	
#define onoff_ch                    Run_Control[0]  //ON/OFF
#define SET_Voltage 	    	        Run_Control[1]  //鐢垫簮璁剧疆鐢靛帇
#define SET_Current	    	    		  Run_Control[2]  //鐢垫簮璁剧疆鐢垫祦
#define SET_Voltage_Laod 					  Run_Control[3]  //鐢靛瓙璐熻浇璁剧疆鐢靛帇
#define SET_Current_Laod					  Run_Control[4]  //鐢靛瓙璐熻浇璁剧疆鐢垫祦
#define LOCK_KEY								    Run_Control[5]  //
#define BEEP_KEY								    Run_Control[6]  //
#define STRONG_KEY							    Run_Control[7]  //

#define MODE_Woke							   	  Run_Control[8]  //
#define POW_Voltage	    	          Run_Control[9]  //
#define Voltage	    	              Run_Control[10]  //
#define Current	    	    		      Run_Control[11]  //
#define Laod_Current	    		      Run_Control[12]  //
#define R_VLUE											Run_Control[13]  //


extern vu16 Contr_Voltage;//
extern vu16 Contr_Current;//
extern vu16 Contr_Laod;//
extern vu16 Correct_Parametet[13];
#define  REG_CorrectionV   Correct_Parametet[0]
#define  REG_CorrectionA   Correct_Parametet[1]
#define  REG_CorrectionA1  Correct_Parametet[2]
#define  REG_CorrectionR   Correct_Parametet[3]
#define  Polar             Correct_Parametet[4]
#define  SET_CorrectionV   Correct_Parametet[5]
#define  SET_CorrectionA   Correct_Parametet[6]
#define  SET_CorrectionA1  Correct_Parametet[7]
#define  CON_CorrectionA   Correct_Parametet[8]
#define  Polar1            Correct_Parametet[9]
#define  Polar2						 Correct_Parametet[10]
#define  Polar3						 Correct_Parametet[11]
#define  REG_CorrectionV1  Correct_Parametet[12]


extern vu32 Correct_Strong[9];//校准系数
#define  REG_ReadV_Offset   Correct_Strong[0]
#define  REG_ReadA_Offset   Correct_Strong[1]
#define  REG_ReadA1_Offset  Correct_Strong[2]
#define  REG_ReadR_Offset   Correct_Strong[3]
#define  SET_ReadV_Offset   Correct_Strong[4]
#define  SET_ReadA_Offset   Correct_Strong[5]
#define  SET_ReadA1_Offset  Correct_Strong[6]
#define  CON_ReadA_Offset   Correct_Strong[7]
#define  REG_ReadV1_Offset  Correct_Strong[8]



//---------------------------
extern vu16 ADC1_Buffer[300];//ADC转换缓冲数组
extern vu16 ADC_NTC_Filt[50];
extern vu16 ADC_Vmon_Filt[50];
extern vu16 ADC_Imon_Filt[50];
extern vu8 UART_Buffer_Rece[16];
extern vu8 UART_Buffer_Send[20];
//============================================================================= 
#define Receive_BUFFERSIZE   10
//=============================================================================
#endif
/******************* (C) COPYRIGHT 2015 KUNKIN *****END OF FILE*************************/