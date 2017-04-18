#include "delay.h"
#include "usart2.h"
#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"
#include "lcd.h"
#include "lcd.h"
#include "motor.h"

//���ڽ��ջ�����
u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; 				//���ջ���,���USART2_MAX_RECV_LEN���ֽ�.
u8 USART2_TX_BUF[USART2_MAX_SEND_LEN]; 			//���ͻ���,���USART2_MAX_SEND_LEN�ֽ�
u8 BLUETOOTH_MESSAGE_BUF[USART2_MAX_RECV_LEN];
//[15]:0,û�н��յ�����;1,���յ���һ������.
//[14:0]:���յ������ݳ���
vu16 BLUETOOTH_MESSAGE_LEN;

//ͨ���жϽ�������2���ַ�֮���ʱ������10ms�������ǲ���һ������������.
//���2���ַ����ռ������10ms,����Ϊ����1����������.Ҳ���ǳ���10msû�н��յ�
//�κ�����,���ʾ�˴ν������.
//���յ�������״̬
//[15]:0,û�н��յ�����;1,���յ���һ������.
//[14:0]:���յ������ݳ���
vu16 USART2_RX_STA=0;
u8	BLUETOOTH_INIT_FLAG;
vu8	CTR_OK;
u8	GETTING_INFO_FLAG;

void _analyse(char * str)
{
	if(BLUETOOTH_INIT_FLAG)
	{
		LCD_Clear(WHITE);
		LCD_Show_Help_str(30,50,16,str);
		if(strstr(str,"OK")||strstr(str,"ERROR:(17)"))
			CTR_OK=TRUE;
	}
	else//�����������ӳɹ�֮��������Ϣ����δ��
	{
		
	}
	USART2_RX_STA=0;//��Ǵ������
}

void USART2_IRQHandler()
{
	u8 res;
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)//���յ�����
	{	 
		res =USART_ReceiveData(USART2);
		if((USART2_RX_STA&(1<<15))==0)//�������һ������,��û�б�����,���ٽ�����������
		{
			if(USART2_RX_STA<USART2_MAX_RECV_LEN)	//�����Խ�������
			{
				USART2_RX_BUF[USART2_RX_STA++]=res;	//��¼���յ���ֵ
				if(res=='\n')
				{
					USART2_RX_BUF[USART2_RX_STA]=0;
					USART2_RX_STA|=1<<15;
					printf("%s",USART2_RX_BUF);//ת����usart1
					if(!GETTING_INFO_FLAG)
						_analyse((char*)USART2_RX_BUF);
					else if(!(BLUETOOTH_MESSAGE_LEN&(1<<15)))
					{
						strcpy((char *)BLUETOOTH_MESSAGE_BUF+BLUETOOTH_MESSAGE_LEN,(char *)USART2_RX_BUF);
						BLUETOOTH_MESSAGE_LEN+=USART2_RX_STA&0x7FFF;
						if(strstr((char *)USART2_RX_BUF,"OK\r\n"))
						{
							BLUETOOTH_MESSAGE_LEN|=1<<15;//��־�������
						}
					}
					USART2_RX_STA=0;
				}
			}else 
			{
				USART2_RX_STA|=1<<15;				//ǿ�Ʊ�ǽ������
				USART2_RX_BUF[USART2_RX_STA&0x7FFF]=0;
				printf("%s",USART2_RX_BUF);//ת����usart1
				if(!GETTING_INFO_FLAG)
					_analyse((char*)USART2_RX_BUF);
				else if(!(BLUETOOTH_MESSAGE_LEN&(1<<15)))
				{
					USART2_RX_BUF[USART2_RX_STA]=0;//��ӽ�����
					strcpy((char *)BLUETOOTH_MESSAGE_BUF+BLUETOOTH_MESSAGE_LEN,(char *)USART2_RX_BUF);
					BLUETOOTH_MESSAGE_LEN+=USART2_RX_STA;
					if(strstr((char *)USART2_RX_BUF,"OK\r\n"))
					{
						BLUETOOTH_MESSAGE_LEN|=1<<15;//��־�������
					}
				}
				USART2_RX_STA=0;
			} 
		}
	}  				 											 
}   


//��ʼ��IO ����2
//pclk1:PCLK1ʱ��Ƶ��(Mhz)
//bound:������
void usart2_init(u32 bound)
{  

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	// GPIOAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE); //����2ʱ��ʹ��

 	USART_DeInit(USART2);  //��λ����2
	//USART2_TX   PA2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��
	
	//USART2_RX	  PA3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PB11
	
	USART_InitStructure.USART_BaudRate = bound;//������һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
  
	USART_Init(USART2, &USART_InitStructure); //��ʼ������	3
	USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ��� 
	//ʹ�ܽ����ж�
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�����ж�
	
	//�����ж����ȼ�
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
	USART2_RX_STA=0;		//����
}

//����3,printf ����
//ȷ��һ�η������ݲ�����USART2_MAX_SEND_LEN�ֽ�
void u2_printf(char* fmt,...)  
{
	u16 i,j; 
	va_list ap; 
	va_start(ap,fmt);
	vsprintf((char*)USART2_TX_BUF,fmt,ap);
	va_end(ap);
	i=strlen((const char*)USART2_TX_BUF);		//�˴η������ݵĳ���
	for(j=0;j<i;j++)							//ѭ����������
	{
	  while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); //ѭ������,ֱ���������   
		USART_SendData(USART2,USART2_TX_BUF[j]); 
	}
}

void init_HC05()
{
	BLUETOOTH_INIT_FLAG=TRUE;
	
	CTR_OK=FALSE;
	while(!CTR_OK)
	{
		u2_printf("AT\r\n");
		delay_ms(1500);
	}
	
	CTR_OK=FALSE;
	while(!CTR_OK)
	{
		u2_printf("AT+INIT\r\n");
		delay_ms(1500);
	}
	
	CTR_OK=FALSE;
	while(!CTR_OK)
	{
		u2_printf("AT+IAC=9E8B33\r\n");
		delay_ms(1500);
	}
	
	CTR_OK=FALSE;
	while(!CTR_OK)
	{
		u2_printf("AT+CLASS=0\r\n");
		delay_ms(1500);
	}
	
	CTR_OK=FALSE;
	while(!CTR_OK)
	{
		u2_printf("AT+INQM=1,5,3\r\n");
		delay_ms(1500);
	}
	
	BLUETOOTH_INIT_FLAG=FALSE;
	GETTING_INFO_FLAG=FALSE;
	BLUETOOTH_MESSAGE_LEN=0;
}

u8 get_bluetooth_info()
{
	if(!GETTING_INFO_FLAG && !BLUETOOTH_INIT_FLAG)
	{
		GETTING_INFO_FLAG=TRUE;
		u2_printf("AT+INQ\r\n");
		return TRUE;
	}
	return FALSE;
}

u8 undo_get_bluetooth_info()
{
	int count=0;
	BLUETOOTH_INIT_FLAG=TRUE;
	CTR_OK=FALSE;
	while(!CTR_OK)
	{
		count++;
		if(count>=10)
			break;
		u2_printf("AT+INQC\r\n");
		delay_ms(1500);
	}
	BLUETOOTH_INIT_FLAG=FALSE;
	return CTR_OK?TRUE:FALSE;
}

