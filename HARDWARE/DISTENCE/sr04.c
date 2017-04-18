#include "sr04.h"
#include "usart.h"
#include "delay.h"
#include "usart3.h"

int distence=0;
u8 GET_DIS_FLAG=0;

///////////////////////////////////////////////TIM4 ���������
u8  TIM4CH1_CAPTURE_STA=0;	//���벶��״̬		    				
u16	TIM4CH1_CAPTURE_VAL;	//���벶��ֵ

void TIM4_Cap_Init(u16 arr,u16 psc)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_ICInitTypeDef  TIM4_ICInitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);  //ʹ��TIM4ʱ��
	
	GPIO_PinRemapConfig(GPIO_Remap_TIM4, ENABLE); //PB6��ӳ��ΪPD12
 
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PD12 ����
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOD,GPIO_Pin_12);
	
	TIM_TimeBaseStructure.TIM_Period = arr; //�趨�������Զ���װֵ 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	//Ԥ��Ƶ��   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_1; //CC1S=01 	ѡ������� IC1ӳ�䵽TI1��
  	TIM4_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
  	TIM4_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
  	TIM4_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
  	TIM4_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 ���������˲��� ���˲�
  	TIM_ICInit(TIM4, &TIM4_ICInitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //��ռ���ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  //�����ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ��� 
	
	TIM_ITConfig(TIM4,TIM_IT_Update|TIM_IT_CC1,ENABLE);//��������ж� ,����CC1IE�����ж�	
	
   	TIM_Cmd(TIM4, ENABLE);//ʹ�ܶ�ʱ��4
}

//��ʱ��4�жϷ������
void TIM4_IRQHandler(void)
{
 	if((TIM4CH1_CAPTURE_STA&0X80)==0)//��δ�ɹ�����	
	{
		if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
		{
			if(TIM4CH1_CAPTURE_STA&0X40)//�Ѿ����񵽸ߵ�ƽ��
			{
				if((TIM4CH1_CAPTURE_STA&0X3F)==0X3F)//�ߵ�ƽ̫����
				{
					TIM4CH1_CAPTURE_STA|=0X80;//��ǳɹ�������һ��
					TIM4CH1_CAPTURE_VAL=0XFFFF;
				}else TIM4CH1_CAPTURE_STA++;
			}
		}
		if (TIM_GetITStatus(TIM4, TIM_IT_CC1) != RESET)//����1���������¼�
		{
			if(TIM4CH1_CAPTURE_STA&0X40)		//����һ���½���
			{
				TIM4CH1_CAPTURE_STA|=0X80;		//��ǳɹ�����һ�θߵ�ƽ����
				TIM4CH1_CAPTURE_VAL=TIM_GetCapture1(TIM4);
		   		TIM_OC1PolarityConfig(TIM4,TIM_ICPolarity_Rising); //CC1P=0 ����Ϊ�����ز���
			}else  								//��δ��ʼ,��һ�β���������
			{
				TIM4CH1_CAPTURE_STA=0;			//���
				TIM4CH1_CAPTURE_VAL=0;
	 			TIM_SetCounter(TIM4,0);
				TIM4CH1_CAPTURE_STA|=0X40;		//��ǲ�����������
		   		TIM_OC1PolarityConfig(TIM4,TIM_ICPolarity_Falling);		//CC1P=1 ����Ϊ�½��ز���
			}
		}
 	}
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC1|TIM_IT_Update); //����жϱ�־λ
}


void SR04_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	TIM4_Cap_Init(0XFFFF,72-1);	//��1Mhz��Ƶ�ʼ���,1us
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	SR04_TRIG=0;						 					//PD11 ���0
	
	distence=0;
	GET_DIS_FLAG=0;
}

//����ֵ������(mm)
int SR04_get_distence(void)
{
	int temp=0;
	u8 count=6;
	
	SR04_TRIG=0;
	delay_us(2);
	SR04_TRIG=1;
	delay_us(10);
	SR04_TRIG=0;
	while(count--)
	{
		delay_ms(10);
		if(TIM4CH1_CAPTURE_STA&0X80)//�ɹ�������һ�θߵ�ƽ
		{
			temp=TIM4CH1_CAPTURE_STA&0X3F;
			temp*=65536;//���ʱ���ܺ�
			temp+=TIM4CH1_CAPTURE_VAL;//�õ��ܵĸߵ�ƽʱ��
			//printf("HIGH:%d us\r\n",temp);//��ӡ�ܵĸߵ�ƽʱ��
			temp=temp*10.0/58.0;
			//printf("%d mm\r\n",temp);//��ӡ�ܵĸߵ�ƽʱ��
			TIM4CH1_CAPTURE_STA=0;//������һ�β���
			break;
		}
	}
	return distence=temp;
}

void send_distence_info()
{
	if(GET_DIS_FLAG)
	{
		u3_printf("%04d mm",distence);
	}
}
