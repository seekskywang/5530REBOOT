/******************** (C) COPYRIGHT 2015 AVER********************
 * 文件名  ：KEY.C
 * 作者    ：李振
 * 描述    ：3*2矩阵键盘扫描
 * 内容    ：扫描完成后出键值
 * 硬件连接：一路3*2矩阵键盘  一路编码器独立按键
 * 修改日期：2015-08-18
********************************************************************/
#include "my_register.h"
#include "beep.h"
#include "flash.h"
#include "gpio.h"
#include "key.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include  "gui.h"
#include "MainTask.h"
#include "ff.h"			/* FatFS文件系统模块*/
#include "usbh_bsp_msc.h"
#include "flash_if.h"

extern WM_HWIN CreateR(void);
extern WM_HWIN CreateWindow2(void);
extern WM_HWIN CreateWindow(void);
//extern WM_HWIN hWinWind;
extern WM_HWIN hWinR;
//extern WM_HWIN load_wind;//璐熻浇鐣岄潰鍙ユ焺
//=================================================================
extern struct bitDefine
{
    unsigned bit0: 1;
    unsigned bit1: 1;
    unsigned bit2: 1;
    unsigned bit3: 1;
    unsigned bit4: 1;
    unsigned bit5: 1;
    unsigned bit6: 1;
    unsigned bit7: 1;
} flagA, flagB,flagC,flagD,flagE,flagF,flagG;
//====================================================================
FRESULT result;
FATFS fs;
FIL file;
//IAP
u8 Jumpflag=0;

UINT fnum;            					  /* 文件成功读写数量 */
BYTE ReadBuffer[1024]={0};        /* 读缓冲区 */
static  u8    filedatabuf[2048];
static  UINT   readcount;

vu8 Flag_SETVGE;
vu8 Flag_SETVSHI;
vu8 Flag_SETVBAI;
vu8 Flag_SETVQIAN;
vu8 Flag_SETAGE;
vu8 Flag_SETASHI;
vu8 Flag_SETABAI;
vu8 Flag_SETAQIAN;

#define MAX_VOL  3200		
#define MAX_CUR  5100	
vu8 t_KeyScan;
vu8 NewKey;
vu8 t_wei;
vu8 t_beep;
vu8 t_Str;
vu8 t_lock;
vu8 t_onoff;
vu8 t_doub;
vu8 t_key_step;//閿洏鎵弿璁℃暟鍣�
vu16 Key1,Key2,Key3,Key4,Key5,Key6,Key7,Key8;
vu32 Key9;
vu16 KeyCounter;
vu16 MAX_limit;//自动测试上限值
vu16 MIN_limit;//自动测试下限值
vu16 Test_Time;
vu16 Test_Time1;
vu16 Time_sw;//定时
vu16 Test_Daley;//自动测试判断等待时间，此时间用户可设置
vu16 Test_C_Door;//自动测试门槛电流
vu8 OFFSET_Zore;//零点值
vu32 Key_Scan(void);
void Lift_Move(void);
void Right_Move(void);
void Setvalue_Add(void);
/* 閿�煎畾涔� */
#define        KEY_1                		 0X4E     //
#define        KEY_2                		 0X56     //
#define        KEY_3                		 0X5E     //
#define        KEY_4                		 0X57      //
#define        KEY_5                		 0X5F      //
#define        KEY_6                		 0X76      //
#define        KEY_7                		 0X67     //
#define        KEY_8                		 0X6F      //
#define        KEY_9                		 0X77     //
#define        KEY_0                		 0X5D      //
#define        KEY_dian              		 0X65     //
#define        KEY_ESC              		 0X6D     //

#define        ENTER                     0X4D    //
#define        PUSH_Lift                 0X45     //
#define        PUSH_Right                0X55      //
#define        PUSH_Up                   0X46      //
#define        PUSH_Down                 0X74      //
#define        PUSH_DISP                 0X47      //
#define        PUSH_SETUP                0X4F     //

#define        KEY_Lock                  0X75     //
#define        KEY_BIAS                  0X6E      //
#define        KEY_Reset                 0X66      //
#define        KEY_TRIG                  0X75      //

#define        KEY_Powe                  0X6C     //
#define        KEY_Face1                 0X64      //
#define        KEY_Face2                 0X5c      //
#define        KEY_Face3                 0X54      //
#define        KEY_Face4                 0X4c      //
#define        KEY_Face5                 0X44      //

/* 瀹氫箟鎸夐敭鍔ㄤ綔 */
#define        KEY_NULL_VALUE             0X00    //无动作
#define        KEY_PRESS                  0X10    //单击
#define        KEY_LONG                   0X20    //长按
#define        KEY_CONTINUE               0X40    //连击
#define        KEY_UP                     0X80    //按键抬起
/*瀹氫箟鎸夐敭澶勭悊姝ラ*/
#define        KEY_INIT_STATE               0      //初态
#define        KEY_WOBBLE_STATE             1      //消抖
#define        KEY_PRESS_STATE              2      //触发单键
#define        KEY_LONG_STATE               3      //长按触发
#define        KEY_CONTINUE_STATE           4      //连续触发
#define        KEY_RELEASE_STATE            5      //恢复初态
/* 闀挎寜鏃堕棿 */
#define         KEY_LONG_PERIOD             50                /* 长按时间1S */
#define        KEY_CONTINUE_PERIOD          10                /* 双击时间500ms */

/************************************************************************************************************************/
vu32 Key_Scan(void)
{
	static vu8 KeyState = KEY_INIT_STATE;
	static vu8 IsKeyRelease = 1;

	static vu8 KeyValueTemp = 0;
	vu8 KeyValue ;
	KeyValue= KEY_NULL_VALUE;
	
	if((NewKey>0x37)&&(IsKeyRelease==1))
	{
		if(KeyState == KEY_INIT_STATE)                    /*无按键按下*/
		{
			KeyState = KEY_WOBBLE_STATE;
			return KEY_NULL_VALUE;
		}
		else if(KeyState == KEY_WOBBLE_STATE)                /* 消抖 */
		{
			KeyState = KEY_PRESS_STATE;
			return KEY_NULL_VALUE;
		}
		else if(KeyState == KEY_PRESS_STATE)                /* 有按键按下，返回按键值 */
		{
			KeyValue = NewKey;
			KeyValueTemp = KeyValue;
			KeyState = KEY_CONTINUE_STATE;
			return KeyValue;
		}
		else if(KeyState == KEY_CONTINUE_STATE)           /*双击按键*/
		{
			KeyCounter ++;
			if(KeyCounter == KEY_CONTINUE_PERIOD)
			{
				 KeyCounter = 0;
				 KeyState = KEY_LONG_STATE;
			}
		}
		else if(KeyState == KEY_LONG_STATE)                        /* 长按按键 */
		{
			KeyCounter ++;
			if(KeyCounter == KEY_LONG_PERIOD)
			{
				KeyCounter = 0;
				Flag_Long=1; //长按触发标志位
			}
		}
	}
	else if(NewKey<=0x37)
	{
		KeyState = KEY_INIT_STATE;        /* 误触发，返回到初始状态 */
		IsKeyRelease = 1;
		Flag_Long=0;
	}
	return KEY_NULL_VALUE;
}

void GetFlashSize(void)
{
    static u16 Stm32_Flash_Size;
    
    Stm32_Flash_Size = (*(__IO u16*)(0x1FFF7A22));//闪存容量寄存器
    
    printf("芯片闪存容量大小为%dK\r\n",Stm32_Flash_Size);

}

void JumpBoot(u8 flag)
{
  	void (*pUserApp)(void);
  uint32_t JumpAddress;
	if(flag==55)
  {		
	__asm("CPSID  I");
        
		JumpAddress = *(volatile uint32_t*) (USER_FLASH_FIRST_PAGE_ADDRESS+4);
		pUserApp = (void (*)(void)) JumpAddress;
		TIM_Cmd(TIM1, DISABLE);	
		TIM_DeInit(TIM1);
		TIM_ITConfig(TIM1,TIM_IT_Update,DISABLE);
		TIM_Cmd(TIM2, DISABLE);	
		TIM_DeInit(TIM2);
		TIM_ITConfig(TIM2,TIM_IT_Update,DISABLE);
		TIM_Cmd(TIM4,DISABLE);
		TIM_DeInit(TIM4);
		TIM_ITConfig(TIM4,TIM_IT_Update,DISABLE);
		TIM_Cmd(TIM6, DISABLE);	
	    TIM_DeInit(TIM6);
		TIM_ITConfig(TIM6,TIM_IT_Update,DISABLE);
		USART_DeInit(USART1);
		USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);		
		USART_Cmd(USART1,DISABLE);
	    USART_DeInit(USART3);
		USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);		
		USART_Cmd(USART3,DISABLE);
		RCC_DeInit();
		RCC_RTCCLKCmd(DISABLE);
		EXTI_DeInit();
		SysTick->CTRL = 0;
		RTC_DeInit();
		RTC_ITConfig(RTC_IT_WUT,DISABLE);//关闭WAKE UP 定时器中断
		RTC_WakeUpCmd( DISABLE);//关闭WAKE UP 定时器　
		__disable_irq();
		NVIC_DisableIRQ(OTG_FS_IRQn);
		NVIC_DisableIRQ(OTG_FS_WKUP_IRQn);
		NVIC_DisableIRQ(OTG_HS_IRQn);
		NVIC_DisableIRQ(OTG_HS_WKUP_IRQn);
		__ASM volatile ("cpsid i");
		/* Initialize user application's Stack Pointer */
		__set_PSP(*(volatile uint32_t*) USER_FLASH_FIRST_PAGE_ADDRESS);
		__set_CONTROL(0);
		__set_MSP(*(volatile uint32_t*) USER_FLASH_FIRST_PAGE_ADDRESS);
		
        
		
//		NVIC_SystemReset();
		pUserApp();
	}
}


uint8_t File_IAP(void)
{
  FRESULT res;
	static u8 signal;
	void (*pUserApp)(void);
  uint32_t JumpAddress;
	u16 i=0;
	u32 wr_state=0;
	static uint32_t fsize;
    result = f_mount(&fs, "2:", 0);
	if(result!=FR_OK)
	{
 //    LCD_ErrLog("f_open->updata.bin->error.\n");
     goto updateerror;
   }
	result=f_open(&file, "2:UPDATE.BIN",FA_OPEN_EXISTING | FA_READ);
	if(result!=FR_OK)
	{
 //    LCD_ErrLog("f_open->updata.bin->error.\n");
     goto updateerror;
  }
//	LCD_UsrLog("f_open->updata.bin->OK!.\n");
	
//	LCD_UsrLog("f_read->updata.bin->..ing.\n");
	fsize=f_size(&file);//unit byte
//	if(fsize>131072)    //128*1024
//	{
//	 // LCD_ErrLog("This firmware is out of limit\n");   
//		while(1)
//		{
//		            //error
//		}
//	}
	//LCD_UsrLog("fsize:%dKB\n",fsize/1024); //因为只擦除了扇区5,所以,固件不能超过128KB
	                                      //如有更大固件需求,请擦除扇区至固件匹配的大小
 // LCD_UsrLog(">Erase.....ing\n");
	if(IAP_Start())
	{ 
	//  LCD_ErrLog(">Erase......error\n");
	  goto updateerror;
	}
//	LCD_UsrLog(">Erase.........ok\n");
	
	while(1)
	{
		do{
		 result=f_read(&file,filedatabuf,2048, &readcount);
     if(result!=FR_OK)
		 {
     //  LCD_UsrLog("f_read->updata.bin->error.\n");
			 goto updateerror;
     }
		 if(signal == 0)
		 {
			 TM1650_SET_LED(0x68,0x70);
			 GPIO_ResetBits(GPIOD,GPIO_Pin_12);//灭灯
			 signal = 1;
		 }else{
			 GPIO_ResetBits(GPIOD,GPIO_Pin_12);
                TM1650_SET_LED(0x48,0x71);
                TM1650_SET_LED(0x68,0xF2);//PASS灯
			 signal = 0;
		 }
		 
		 wr_state=IAP_Flash_Write(USER_FLASH_FIRST_PAGE_ADDRESS+i*2048,filedatabuf,readcount/4);
		 if(0!=wr_state) 
		 {
			// LCD_UsrLog("Flash.........error\n");
			 goto updateerror;
		 } 
		 i++;
	 }while(f_eof(&file)==0);//到达文件末尾

	//  LCD_UsrLog("ffok...............%dKB\n",fsize/1024);

   // LCD_UsrLog("ffok............COMPLETE\n");
	 // LCD_UsrLog("Jump to the application...\n");
	  Jumpflag = 55;
//	  I2C_EE_BufferWrite(&Jumpflag, EEP_Firstpage, 1);
	  goto updateerror;
  }

	  updateerror:
	  __asm("CPSID  I");
        
		JumpAddress = *(volatile uint32_t*) (USER_FLASH_FIRST_PAGE_ADDRESS+4);
		pUserApp = (void (*)(void)) JumpAddress;
		TIM_Cmd(TIM1, DISABLE);	
		TIM_DeInit(TIM1);
		TIM_ITConfig(TIM1,TIM_IT_Update,DISABLE);
		TIM_Cmd(TIM2, DISABLE);	
		TIM_DeInit(TIM2);
		TIM_ITConfig(TIM2,TIM_IT_Update,DISABLE);
		TIM_Cmd(TIM4,DISABLE);
		TIM_DeInit(TIM4);
		TIM_ITConfig(TIM4,TIM_IT_Update,DISABLE);
		TIM_Cmd(TIM6, DISABLE);	
	    TIM_DeInit(TIM6);
		TIM_ITConfig(TIM6,TIM_IT_Update,DISABLE);
		USART_DeInit(USART1);
		USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);		
		USART_Cmd(USART1,DISABLE);
	    USART_DeInit(USART3);
		USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);		
		USART_Cmd(USART3,DISABLE);
		RCC_DeInit();
		RCC_RTCCLKCmd(DISABLE);
		EXTI_DeInit();
		SysTick->CTRL = 0;
		RTC_DeInit();
		RTC_ITConfig(RTC_IT_WUT,DISABLE);//关闭WAKE UP 定时器中断
		RTC_WakeUpCmd( DISABLE);//关闭WAKE UP 定时器　
		__disable_irq();
		NVIC_DisableIRQ(OTG_FS_IRQn);
		NVIC_DisableIRQ(OTG_FS_WKUP_IRQn);
		NVIC_DisableIRQ(OTG_HS_IRQn);
		NVIC_DisableIRQ(OTG_HS_WKUP_IRQn);
		__ASM volatile ("cpsid i");
		/* Initialize user application's Stack Pointer */
		__set_PSP(*(volatile uint32_t*) USER_FLASH_FIRST_PAGE_ADDRESS);
		__set_CONTROL(0);
		__set_MSP(*(volatile uint32_t*) USER_FLASH_FIRST_PAGE_ADDRESS);
		
        
		
//		NVIC_SystemReset();
		pUserApp();
		/*************************Jump to User Application area end***************************/
	
    return 0;

}

/***************************************************************************************************************************/
void Key_Funtion(void)
{
	vu32 KeyValue = 0;
	static vu8 t_KEYON;
	static vu8 a,b;
/****************************************************鐭寜鍔熻兘************************************************************/
	if(Flag_Key_Scan==1)
	{
		Flag_Key_Scan = 0;
		KeyValue = Key_Scan();
		switch(KeyValue)
		{
			case KEY_1 ://
			{
				SET_Voltage=1000;
				SET_Current=4000;
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
			}
			break;
			
			case KEY_2 ://
			{
				SET_Voltage=420;
				SET_Current=1000;
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
			}
			break;
			
			case KEY_3 ://
			{
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
			}
			break;
			
			case KEY_4 ://
			{
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
			}
			break;
			
			case KEY_5 ://
			{
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
			}
			break;	
			
			case KEY_6 ://
			{
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
			}
			break;
			
			case KEY_7 ://
			{
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
			}
			break;
			
			case KEY_8 ://
			{
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
			}
			break;
			
			case KEY_9 ://
			{
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
			}
			break;
			
			case KEY_0 ://
			{
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
			}
			break;
			
			case KEY_dian ://
			{
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
			}
			break;
			
			case KEY_ESC :
			{
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
			}
			break;
			
			case PUSH_Lift :
			{
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
			}
			break;
			case PUSH_Right :
			{
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
			}
			break;
			case PUSH_Up :
			{
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
				SET_Current_Laod=SET_Current_Laod+1;
			}
			break;
			case PUSH_Down :
			{
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
				SET_Current_Laod=SET_Current_Laod-1;
			}
			break;
			
			case ENTER ://
			{
				File_IAP();
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
			}
			break;
			case PUSH_DISP :
			{
				
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
			}
			break;
			case PUSH_SETUP :
			{
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�

				t_onoff = 1;
			}
			break;
			case KEY_TRIG:
			{
//				t_onoff++;
//				if(t_onoff>1)
//				{
//					t_onoff=0;
//				}
//				if(t_onoff==0)
//				{
//					Flag_Swtich_ON=0;
//					GPIO_SetBits(GPIOC,GPIO_Pin_1);//OFF
//					
//				}
//				else if(t_onoff==1)
//				{
//					Flag_Swtich_ON=1;
//					GPIO_ResetBits(GPIOC,GPIO_Pin_1);//On
//				}
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
			}
			break;
			case KEY_Reset:
			{
//				static vu8 POW_t;
//				POW_t++;
//				if(POW_t>1)
//				{
//					POW_t=0;
//				}
//				if(POW_t==0)
//				{
//					GPIO_ResetBits(GPIOB,GPIO_Pin_13);//鍏抽棴绋冲帇鐢垫簮杈撳嚭
//				}
//				else if(POW_t==1)
//				{
//					GPIO_SetBits(GPIOB,GPIO_Pin_13);//鎵撳紑绋冲帇鐢垫簮杈撳嚭
//				}
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
			}
			break;
			case KEY_Powe :
			{
				
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
			}
			break;
			case KEY_Face1 :
			{
//				WM_DeleteWindow(hWinWind);
				WM_DeleteWindow(hWinR);
				CreateR();
				Mode_SW_CONT(0x01);//杩涘叆鍐呴樆娴嬭瘯妯″紡
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
			}
			break;
			case KEY_Face2 :
			{
//				WM_DeleteWindow(hWinR);
//				WM_DeleteWindow(hWinWind);
//				CreateWindow2();
//				Mode_SW_CONT(0x02);//杩涘叆璐熻浇妯″紡
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
			}
			break;
			case KEY_Face3 :
			{
//				WM_DeleteWindow(hWinR);
//				WM_DeleteWindow(hWinWind);
//				CreateWindow();
//				Mode_SW_CONT(0x03);//杩涘叆鐢垫簮妯″紡
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
			}
			break;
			case KEY_Face4 :
			{
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
			}
			break;
			case KEY_Face5 :
			{
				KeyCounter = 0;
				BEEP_Tiggr();//瑙﹀彂铚傞福鍣�
			}
			break;
			
			default:
			break;
		}	
	}
}
/***************************************************************************************************************************/
void Lift_Move(void)
{
	t_wei++;
	if(t_wei>3)
	{
		t_wei=0;
	}
	if(t_wei==0)
	{
		Flag_QIAN=1;
		Flag_GE=0;
		Flag_SHI=0;
		Flag_BAI=0;
	}
	else if(t_wei==1)
	{
		Flag_GE=1;
		Flag_QIAN=0;
		Flag_SHI=0;
		Flag_BAI=0;
	}
	else if(t_wei==2)
	{
		Flag_SHI=1;
		Flag_QIAN=0;
		Flag_GE=0;
		Flag_BAI=0;
	}
	else if(t_wei==3)
	{
		Flag_BAI=1;
		Flag_QIAN=0;
		Flag_GE=0;
		Flag_SHI=0;
	}
}
/***********************************************************************************************************************/
void Right_Move(void)
{
	t_wei--;
	Flag_DisSet=1;//开启设置值显示
	if(t_wei>3)
	{
		t_wei=3;
	}
	if(t_wei==0)
	{
		Flag_QIAN=1;
		Flag_GE=0;
		Flag_SHI=0;
		Flag_BAI=0;
	}
	else if(t_wei==1)
	{
		Flag_GE=1;
		Flag_QIAN=0;
		Flag_SHI=0;
		Flag_BAI=0;
	}
	else if(t_wei==2)
	{
		Flag_SHI=1;
		Flag_QIAN=0;
		Flag_GE=0;
		Flag_BAI=0;
	}
	else if(t_wei==3)
	{
		Flag_BAI=1;
		Flag_QIAN=0;
		Flag_GE=0;
		Flag_SHI=0;
	}
}
/*************************************************************************************************************************/
void Setvalue_Add(void)//设置电压和电流递增
{
	/********电压设置*********************/
	if(Flag_GE==1&&Flag_SetV==1)
	{
		SET_Voltage++;
		if(SET_Voltage>MAX_VOL)
		{
			SET_Voltage=MAX_VOL;
		}
	}
	else if(Flag_SHI==1&&Flag_SetV==1)
	{
		SET_Voltage=SET_Voltage+10;
		if(SET_Voltage>MAX_VOL)
		{
			SET_Voltage=MAX_VOL;//如果数值大于3000本次递增无效
		}
	}
	else if(Flag_BAI==1&&Flag_SetV==1)
	{
		SET_Voltage=SET_Voltage+100;
		if(SET_Voltage>MAX_VOL)
		{
			SET_Voltage=MAX_VOL;//如果数值大于3000本次递增无效
		}
	}
	else if(Flag_QIAN==1&&Flag_SetV==1)
	{
		SET_Voltage=SET_Voltage+1000;
		if(SET_Voltage>MAX_VOL)
		{
			SET_Voltage=SET_Voltage-1000;//如果数值大于3000本次递增无效
		}
	}
/**********电流设置*******************/
	if(Flag_GE==1&&Flag_SetA==1)
	{
		if(flag_Test_Min==1)//设置下限值
		{
			MIN_limit++;
			if( MIN_limit>MAX_CUR)
			{
				 MIN_limit=MAX_CUR;
			}
		}
		else if(flag_Test_MAX==1)//设置上限值
		{
			MAX_limit++;
			if( MAX_limit>MAX_CUR)
			{
				 MAX_limit=MAX_CUR;
			}
		}
		else if(flag_Test_SetTime==1)//设置测试时间
		{
			Test_Daley++;
			if( Test_Daley>9999)
			{
				Test_Daley=9999;
			}
		}
		else if(flag_Test_Door==1)//设置门槛电流
		{
			Test_C_Door++;
			if(Test_C_Door>MAX_CUR)
			{
				Test_C_Door=MAX_CUR;
			}
		}
		else if(flag_ADJ_ON==1)//进入校准模式
		{
			ADJ_Write++;
			if(ADJ_Write>MAX_CUR)
			{
				ADJ_Write=MAX_CUR;
			}
		}
		else
		{
			SET_Current++;
			if(SET_Current>MAX_CUR)
			{
				SET_Current=MAX_CUR;
			}
		}
	}
	else if(Flag_SHI==1&&Flag_SetA==1)
	{
		if(flag_Test_Min==1)//设置下限值
		{
			MIN_limit=MIN_limit+10;
			if( MIN_limit>MAX_CUR)
			{
				 MIN_limit=MIN_limit-10;
			}
		}
		else if(flag_Test_MAX==1)//设置上限值
		{
			MAX_limit=MAX_limit+10;
			if( MAX_limit>MAX_CUR)
			{
				 MAX_limit=MAX_limit-10;
			}
		}
		else if(flag_Test_SetTime==1)//设置测试时间
		{
			Test_Daley=Test_Daley+10;
			if( Test_Daley>9999)
			{
				Test_Daley=Test_Daley-10;
			}
		}
		else if(flag_Test_Door==1)//设置门槛电流
		{
			Test_C_Door=Test_C_Door+10;
			if(Test_C_Door>MAX_CUR)
			{
				Test_C_Door=Test_C_Door-10;
			}
		}
		else if(flag_ADJ_ON==1)//进入校准模式
		{
			ADJ_Write=ADJ_Write+10;
			if(ADJ_Write>MAX_CUR)
			{
				ADJ_Write=MAX_CUR;//如果数值大于5000本次递增无效
			}
		}
		else
		{
			SET_Current=SET_Current+10;
			if(SET_Current>MAX_CUR)
			{
				SET_Current=MAX_CUR;//如果数值大于5000本次递增无效
			}
		}
	}
	else if(Flag_BAI==1&&Flag_SetA==1)
	{
		if(flag_Test_Min==1)//设置下限值
		{
			MIN_limit=MIN_limit+100;
			if( MIN_limit>MAX_CUR)
			{
				 MIN_limit=MIN_limit-100;
			}
		}
		else if(flag_Test_MAX==1)//设置上限值
		{
			MAX_limit=MAX_limit+100;
			if( MAX_limit>MAX_CUR)
			{
				 MAX_limit=MAX_limit-100;
			}
		}
		else if(flag_Test_SetTime==1)//设置测试时间
		{
			Test_Daley=Test_Daley+100;
			if( Test_Daley>9999)
			{
				Test_Daley=Test_Daley-100;
			}
		}
		else if(flag_Test_Door==1)//设置门槛电流
		{
			Test_C_Door=Test_C_Door+100;
			if(Test_C_Door>MAX_CUR)
			{
				Test_C_Door=Test_C_Door-100;
			}
		}
		else if(flag_ADJ_ON==1)//进入校准模式
		{
			ADJ_Write=ADJ_Write+100;
			if(ADJ_Write>MAX_CUR)
			{
				ADJ_Write=ADJ_Write-100;
			}
		}
		else
		{
			SET_Current=SET_Current+100;
			if(SET_Current>MAX_CUR)
			{
				SET_Current=MAX_CUR;//如果数值大于5000本次递增无效
			}
		}
	}
	else if(Flag_QIAN==1&&Flag_SetA==1)
	{
		if(flag_Test_Min==1)//设置下限值
		{
			MIN_limit=MIN_limit+1000;
			if( MIN_limit>MAX_CUR)
			{
				 MIN_limit=MIN_limit-1000;
			}
		}
		else if(flag_Test_MAX==1)//设置上限值
		{
			MAX_limit=MAX_limit+1000;
			if( MAX_limit>MAX_CUR)
			{
				 MAX_limit=MAX_limit-1000;
			}
		}
		else if(flag_Test_SetTime==1)//设置测试时间
		{
			Test_Daley=Test_Daley+1000;
			if( Test_Daley>9999)
			{
				Test_Daley=Test_Daley-1000;
			}
		}
		else if(flag_Test_Door==1)//设置门槛电流
		{
			Test_C_Door=Test_C_Door+1000;
			if(Test_C_Door>MAX_CUR)
			{
				Test_C_Door=Test_C_Door-1000;
			}
		}
		else if(flag_ADJ_ON==1)//进入校准模式
		{
			ADJ_Write=ADJ_Write+1000;
			if(ADJ_Write>MAX_CUR)
			{
				ADJ_Write=ADJ_Write-1000;//如果数值大于5000本次递增无效
			}
		}
		else
		{
			SET_Current=SET_Current+1000;
			if(SET_Current>MAX_CUR)
			{
				SET_Current=SET_Current-1000;//如果数值大于5000本次递增无效
			}
		}
	}
}
/***************************************************************************************/
void Setvalue_Reduction(void)//设置电压和电流递减
{
	/*********************设置电压***********************************************/
	if(Flag_GE==1&&Flag_SetV==1)
	{
		SET_Voltage--;
		if(SET_Voltage>MAX_VOL)
		{
			SET_Voltage=0;
		}
	}
	else if(Flag_SHI==1&&Flag_SetV==1)
	{
		SET_Voltage=SET_Voltage-10;
		if(SET_Voltage>MAX_VOL)
		{
			SET_Voltage=SET_Voltage+10;//如果数值大于3000本次递减无效
		}
	}
	else if(Flag_BAI==1&&Flag_SetV==1)
	{
		SET_Voltage=SET_Voltage-100;
		if(SET_Voltage>MAX_VOL)
		{
			SET_Voltage=SET_Voltage+100;//如果数值大于3000本次递减无效
		}
	}
	else if(Flag_QIAN==1&&Flag_SetV==1)
	{
		SET_Voltage=SET_Voltage-1000;
		if(SET_Voltage>MAX_VOL)
		{
			SET_Voltage=SET_Voltage+1000;//如果数值大于3000本次递减无效
		}
	}
/******************************设置电流******************************************/
	if(Flag_GE==1&&Flag_SetA==1)
	{
		if(flag_Test_Min==1)//设置下限值
		{
			MIN_limit--;
			if( MIN_limit>MAX_CUR)
			{
				 MIN_limit=0;
			}
		}
		else if(flag_Test_MAX==1)//设置上限值
		{
			MAX_limit--;
			if( MAX_limit>MAX_CUR)
			{
				 MAX_limit=0;
			}
		}
		else if(flag_Test_SetTime==1)//设置测试时间
		{
			Test_Daley--;
			if( Test_Daley>9999)
			{
				Test_Daley=0;
			}
		}
		else if(flag_Test_Door==1)//设置门槛电流
		{
			Test_C_Door--;
			if(Test_C_Door<10)
			{
				Test_C_Door=10;
			}
		}
		else if(flag_ADJ_ON==1)//进入校准模式
		{
			ADJ_Write--;//设置电流值
			if(ADJ_Write>MAX_CUR)
			{
				ADJ_Write=0;
			}
		}
		else
		{
			SET_Current--;//设置电流值
			if(SET_Current>MAX_CUR)
			{
				SET_Current=0;
			}
		}
		
	}
	else if(Flag_SHI==1&&Flag_SetA==1)
	{
		if(flag_Test_Min==1)//设置下限值
		{
			MIN_limit=MIN_limit-10;
			if( MIN_limit>MAX_CUR)
			{
				 MIN_limit=MIN_limit+10;
			}
		}
		else if(flag_Test_MAX==1)//设置上限值
		{
			MAX_limit=MAX_limit-10;
			if( MAX_limit>MAX_CUR)
			{
				 MAX_limit=MAX_limit+10;
			}
		}
		else if(flag_Test_SetTime==1)//设置测试时间
		{
			Test_Daley=Test_Daley-10;
			if( Test_Daley>9999)
			{
				Test_Daley=Test_Daley+10;
			}
		}
		else if(flag_Test_Door==1)//设置门槛电流
		{
			Test_C_Door=Test_C_Door-10;
			if(Test_C_Door>MAX_CUR)
			{
				Test_C_Door=Test_C_Door+10;
			}
		}
		else if(flag_ADJ_ON==1)//进入校准模式
		{
			ADJ_Write=ADJ_Write-10;
			if(ADJ_Write>MAX_CUR)
			{
				ADJ_Write=ADJ_Write+10;//如果数值大于5000本次递减无效
			}
		}
		else
		{
			SET_Current=SET_Current-10;
			if(SET_Current>MAX_CUR)
			{
				SET_Current=SET_Current+10;//如果数值大于5000本次递减无效
			}
		}
	}
	else if(Flag_BAI==1&&Flag_SetA==1)
	{
		if(flag_Test_Min==1)//设置下限值
		{
			MIN_limit=MIN_limit-100;
			if( MIN_limit>MAX_CUR)
			{
				 MIN_limit=MIN_limit+100;
			}
		}
		else if(flag_Test_MAX==1)//设置上限值
		{
			MAX_limit=MAX_limit-100;
			if( MAX_limit>MAX_CUR)
			{
				 MAX_limit=MAX_limit+100;
			}
		}
		else if(flag_Test_SetTime==1)//设置测试时间
		{
			Test_Daley=Test_Daley-100;
			if( Test_Daley>9999)
			{
				Test_Daley=Test_Daley+100;
			}
		}
		else if(flag_Test_Door==1)//设置门槛电流
		{
			Test_C_Door=Test_C_Door-100;
			if(Test_C_Door>MAX_CUR)
			{
				Test_C_Door=Test_C_Door+100;
			}
		}
		else if(flag_ADJ_ON==1)//进入校准模式
		{
			ADJ_Write=ADJ_Write-100;
			if(ADJ_Write>MAX_CUR)
			{
				ADJ_Write=ADJ_Write+100;//如果数值大于5000本次递减无效
			}
		}
		else
		{
			SET_Current=SET_Current-100;
			if(SET_Current>MAX_CUR)
			{
				SET_Current=SET_Current+100;//如果数值大于5000本次递减无效
			}
		}
	}
	else if(Flag_QIAN==1&&Flag_SetA==1)
	{
		if(flag_Test_Min==1)//设置下限值
		{
			MIN_limit=MIN_limit-1000;
			if( MIN_limit>MAX_CUR)
			{
				 MIN_limit=MIN_limit+1000;
			}
		}
		else if(flag_Test_MAX==1)//设置上限值
		{
			MAX_limit=MAX_limit-1000;
			if( MAX_limit>MAX_CUR)
			{
				 MAX_limit=MAX_limit+1000;
			}
		}
		else if(flag_Test_SetTime==1)//设置测试时间
		{
			Test_Daley=Test_Daley-1000;
			if( Test_Daley>9999)
			{
				Test_Daley=Test_Daley+1000;
			}
		}
		else if(flag_Test_Door==1)//设置门槛电流
		{
			Test_C_Door=Test_C_Door-1000;
			if(Test_C_Door>MAX_CUR)
			{
				Test_C_Door=Test_C_Door+1000;
			}
		}
		else if(flag_ADJ_ON==1)//进入校准模式
		{
			ADJ_Write=ADJ_Write-1000;
			if(ADJ_Write>MAX_CUR)
			{
				ADJ_Write=ADJ_Write+1000;//如果数值大于5000本次递减无效
			}
		}
		else
		{
			SET_Current=SET_Current-1000;
			if(SET_Current>MAX_CUR)
			{
				SET_Current=SET_Current+1000;//如果数值大于5000本次递减无效
			}
		}
	}
}
/********************************************************************************/
void Mode_SW_CONT(vu8 mode)//模式切换控制
{
	switch(mode)
	{
		case 0x01 ://内阻测试模式
		{
			GPIO_ResetBits(GPIOE,GPIO_Pin_2);
			GPIO_ResetBits(GPIOE,GPIO_Pin_3);
		}
		break;
		case 0x02 ://负载模式
		{
			GPIO_SetBits(GPIOE,GPIO_Pin_2);
			GPIO_ResetBits(GPIOE,GPIO_Pin_3);
		}
		break;
		case 0x03 ://直流电源模式
		{
			GPIO_ResetBits(GPIOE,GPIO_Pin_2);//
			GPIO_SetBits(GPIOE,GPIO_Pin_3);//
		}
		break;
		default:
		break;
	}
}
