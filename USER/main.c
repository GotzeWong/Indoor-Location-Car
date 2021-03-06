#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"
#include "usart3.h"
#include "usart2.h"
#include "w25qxx.h"
#include "vs10xx.h"
#include "malloc.h"
#include "exfuns.h"
#include "fontupd.h"
#include "motor.h"
#include "beep.h"
#include "sr04.h"
#include "mp3player.h"
#include "servo.h"
#include "mpu6050.h"
#include "inv_mpu.h"

void init()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	delay_init();	    	 //延时函数初始化
	uart_init(115200);	 	//串口初始化为115200
	usart2_init(38400);//蓝牙AT模式波特率38400，通信模式9600
	usart3_init(115200);
	LED_Init();			     //LED端口初始化
	BEEP_Init();
	LCD_Init();
	W25QXX_Init();				//初始化W25Q128
	VS_Init();	  				//初始化VS1053 
//	SR04_Init();
//	Servo_Init();
 	
 	my_mem_init(SRAMIN);		//初始化内部内存池
	exfuns_init();				//为fatfs相关变量申请内存  
	f_mount(fs[0],"0:",1); 		//挂载SD卡 
 	f_mount(fs[1],"1:",1); 		//挂载FLASH.
	
	while(font_init()) 				//检查字库
	{
		LCD_ShowString(30,50,200,16,16,"Font Error!");
		delay_ms(200);
		LCD_Fill(30,50,240,66,WHITE);//清除显示
	}
//	init_HC05();
	//init_wifi_sta_client_trans();
	car_init();
	car_set_speed(4);
	VS_Set_Volum(4);
	mp3_init();
	
	while(MPU_Init())					//初始化MPU6050
	{
		LCD_ShowString(30,130,200,16,16,"MPU6050 init Error");
		delay_ms(200);
		LCD_Fill(30,130,239,130+16,WHITE);
 		delay_ms(200);
	}
	while(mpu_dmp_init())
 	{
		LCD_ShowString(30,130,200,16,16,"MPU6050 dmp Error");
		delay_ms(200);
		LCD_Fill(30,130,239,130+16,WHITE);
 		delay_ms(200);
	}  
}

int main()
{
	char str[20];
	u32 t=0;
	
	init();
	
	//初始化完成，滴一声
	BEEP_DI();
	
	while(1)
	{
		//接收bluetooth消息完成，发送给服务器
//		send_bluetooth_info();
		
		//播放音乐部分
		mp3_play();
		
		//超声波检测距离并发送
//		if(GET_DIS_FLAG)
//		{
//			SR04_get_distence();
//			send_distence_info();
//			GET_DIS_FLAG=0;
//		}
		
		if(UPDATE_OLA_FLAG)
		{
			LED0=!LED0;
			if(mpu_dmp_get_data(&pitch,&roll,&yaw)==0)//100000次用时45s   读的频率必须高一点。。。
			{
				MPU_Get_Accelerometer(&aacx,&aacy,&aacz);	//得到加速度传感器数据
				MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz);	//得到陀螺仪数据
				//u3_printf();
			}
			UPDATE_OLA_FLAG=0;
		}
		
		if(t>=100000)
		{
			LED1=!LED1;
			sprintf(str,"yaw: %f\n roll: %f\n pitch: %f\n left: %hd\n right: %hd",yaw,roll,pitch,TIM3->CCR1,TIM3->CCR2);
			LCD_Show_Help_str(30,50,16,str);//参数y=-1代表显示在前一条后面一行
			sprintf(str,"ACC\r\n  x:%f\r\n  y:%f\r\n  z:%f\r\n",9.8*aacx*1.0/16384.0,9.8*aacy*1.0/16384.0,9.8*aacz*1.0/16384.0);
			LCD_Show_Help_str(30,-1,16,str);//参数y=-1代表显示在前一条后面一行
			sprintf(str,"GYR\nx:%d\ny:%d\nz:%d",gyrox,gyroy,gyroz);
			LCD_Show_Help_str(30,-1,16,str);//参数y =-1代表显示在前一条后面一行
			t=0;
		}
		t++;
	}
}
