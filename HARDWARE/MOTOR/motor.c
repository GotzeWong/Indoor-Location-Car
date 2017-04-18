#include "motor.h"
#include "inv_mpu.h"

//speed ��������
u8 SPEED_DIF=5;
u8 MAX_DEGREE=90;
u8 START_AUST=2;

#define STOP	0
#define FORWARD 1
#define BACK	2
#define LEFT	3
#define RIGHT	4

//ת���ٶ�
#define TURN_SPEED 120

u8 cur_task=STOP;//��ǰ״̬
float raw_yaw=0;//������ʼǰ�ĺ����
float tar_yaw=0;//Ŀ�ĺ����

//��TIM3 ������ֵΪ255
int speed=150;//Լ80<=speed<=255ת

u8 get_speed(int speed,int l,int r)
{
	if(speed<l)
		return l;
	if(speed>r)
		return r;
	return speed;
}

void TIM3_PWM_Init(u16 arr,u16 psc)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	//�⺯����ֹJTAG����
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	//ʹ�ܶ�ʱ��3ʱ��
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC  | RCC_APB2Periph_AFIO, ENABLE);  //ʹ��GPIO�����AFIO���ù���ģ��ʱ��
	
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE); //Timer3��ȫ��ӳ��  TIM3_CH2->Pc7  TIM3_CH1->Pc6
 
   //���ø�����Ϊ�����������,���TIM3 CH1 CH2��PWM���岨��	TIM3_CH2->Pc7  TIM3_CH1->Pc6
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //TIM_CH2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; //TIM_CH2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIO

   //��ʼ��TIM3
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	
	//��ʼ��TIM3 Channel 1,2 PWMģʽ
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM������ȵ���ģʽ2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //�������:TIM����Ƚϼ��Ը�
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);  //����Tָ���Ĳ�����ʼ������TIM3 OC2
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM������ȵ���ģʽ2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //�������:TIM����Ƚϼ��Ը�
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);  //����Tָ���Ĳ�����ʼ������TIM3 OC2

	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);  //ʹ��TIM3��CCR2�ϵ�Ԥװ�ؼĴ���
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);

	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIM3
}

void car_forward(){
	
	cur_task=FORWARD;
	raw_yaw=tar_yaw=yaw;
	
	TIM_Cmd(TIM5,ENABLE);//������ʱ��5
	MOTOR_L_F=1;
	MOTOR_L_B=0;
	MOTOR_R_F=1;
	MOTOR_R_B=0;
	TIM_SetCompare1(TIM3,speed);
	TIM_SetCompare2(TIM3,speed);
}
void car_back(){
	
	cur_task=BACK;
	raw_yaw=tar_yaw=yaw;
	
	TIM_Cmd(TIM5,ENABLE);//������ʱ��5
	MOTOR_L_F=0;
	MOTOR_L_B=1;
	MOTOR_R_F=0;
	MOTOR_R_B=1;
	TIM_SetCompare1(TIM3,speed);
	TIM_SetCompare2(TIM3,speed);
}
void turn_left()
{
	MOTOR_L_F=0;
	MOTOR_L_B=1;
	MOTOR_R_F=1;
	MOTOR_R_B=0;
	TIM_SetCompare1(TIM3,TURN_SPEED);
	TIM_SetCompare2(TIM3,TURN_SPEED);
}
void turn_right()
{
	MOTOR_L_F=1;
	MOTOR_L_B=0;
	MOTOR_R_F=0;
	MOTOR_R_B=1;
	TIM_SetCompare1(TIM3,TURN_SPEED);
	TIM_SetCompare2(TIM3,TURN_SPEED);
}
void car_left(u8 degree){
	cur_task=LEFT;
	raw_yaw=yaw;
	tar_yaw=raw_yaw+degree;
	if(tar_yaw>180)
	{
		tar_yaw-=360;
	}
	
	TIM_Cmd(TIM5,ENABLE);//������ʱ��5
}
void car_right(u8 degree){
	cur_task=RIGHT;
	raw_yaw=yaw;
	tar_yaw=raw_yaw-degree;
	if(tar_yaw<-180)
	{
		tar_yaw+=360;
	}
	
	TIM_Cmd(TIM5,ENABLE);//������ʱ��5
}
void car_set_speed(int s)
{
	s=(255-80)*1.0*s/10.0+80;
	speed=get_speed(s,80,255);
	TIM_SetCompare1(TIM3,speed);
	TIM_SetCompare2(TIM3,speed);
}
void car_slow(){
	speed-=SPEED_STEP;
	speed=get_speed(speed,80,255);
	TIM_SetCompare1(TIM3,speed);
	TIM_SetCompare2(TIM3,speed);
}
void car_fast(){
	speed+=SPEED_STEP;
	speed=get_speed(speed,80,255);
	TIM_SetCompare1(TIM3,speed);
	TIM_SetCompare2(TIM3,speed);
}

void car_stop(void){
	
	cur_task=STOP;
	raw_yaw=tar_yaw=yaw;
	
	TIM_Cmd(TIM5,DISABLE);//�رն�ʱ��5
	MOTOR_L_F=0;
	MOTOR_L_B=0;
	MOTOR_R_F=0;
	MOTOR_R_B=0;
	TIM_SetCompare1(TIM3,0);
	TIM_SetCompare2(TIM3,0);
}

void left_more()//����΢��
{
	if(TIM3->CCR2-TIM3->CCR1<MAX_DEGREE)
	{
		TIM3->CCR1=get_speed(TIM3->CCR1-SPEED_DIF,80,255);//���ּ�Сһ���ٶ�
		TIM3->CCR2=get_speed(TIM3->CCR2+SPEED_DIF,80,255);//��������һ���ٶ�
	}
}

void right_more()//����΢��
{
	if(TIM3->CCR1-TIM3->CCR2<MAX_DEGREE)
	{
		TIM3->CCR1=get_speed(TIM3->CCR1+SPEED_DIF,80,255);//��������һ���ٶ�
		TIM3->CCR2=get_speed(TIM3->CCR2-SPEED_DIF,80,255);//���ּ�Сһ���ٶ�
	}
}

//��ʱ��5�жϷ������		    
void TIM5_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)//�Ǹ����ж�
	{
		//todo
		switch(cur_task)
		{
			case STOP:
				TIM_Cmd(TIM5,DISABLE);//�رն�ʱ��5
				break;
			case FORWARD:
				if(yaw>tar_yaw+START_AUST)
					right_more();
				else if(yaw<tar_yaw-START_AUST)
					left_more();
				break;
			case LEFT:
				if(yaw>tar_yaw+START_AUST)
					turn_right();
				else if(yaw<tar_yaw-START_AUST)
					turn_left();
				else
					car_stop();
			case RIGHT:
				if(yaw>tar_yaw+START_AUST)
					turn_right();
				else if(yaw<tar_yaw-START_AUST)
					turn_left();
				else
					car_stop();
				break;
			case BACK:
				if(yaw>tar_yaw+START_AUST)
					left_more();
				else if(yaw<tar_yaw-START_AUST)
					right_more();
				break;
			default:
				break;
		}
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update  );  //���TIM5�����жϱ�־    
	}
}

void TIM5_Int_Init(u16 arr,u16 psc)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);//TIM5ʱ��ʹ��    
	
	//��ʱ��TIM5��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIM5�ж�,���������ж�
	
	TIM_Cmd(TIM5,ENABLE);//������ʱ��5
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
}

//��ʼ��PF1 2 3 4
//С��IO��ʼ��
void car_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	TIM3_PWM_Init(255,573);	 //PWMƵ��=72000000/((255+1)*(573+1))ԼΪ490HZ����arduinoһ��

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);	 //ʹ��PF�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;						//LED0-->PB.5 �˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOF, &GPIO_InitStructure);					 		//�����趨������ʼ��GPIOB.5

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	TIM5_Int_Init(1000-1,7200-1);//100ms
	car_stop();
}
