#ifndef __USART2_H
#define __USART2_H	 
#include "sys.h"   

#ifndef FALSE

#define FALSE	0
#define TRUE	1

#endif

#define USART2_MAX_RECV_LEN		200					//�����ջ����ֽ���
#define USART2_MAX_SEND_LEN		100					//����ͻ����ֽ���
#define USART2_RX_EN 			1					//0,������;1,����.

//#define BLUETOOTH_AT_MODEL PAout(4)// PA4
//#define BLUETOOTH_CS PAout(5)// PA5 ENABLE

extern u8  USART2_RX_BUF[USART2_MAX_RECV_LEN]; 		//���ջ���,���USART3_MAX_RECV_LEN�ֽ�
extern u8  USART2_TX_BUF[USART2_MAX_SEND_LEN]; 		//���ͻ���,���USART3_MAX_SEND_LEN�ֽ�
extern u8 BLUETOOTH_MESSAGE_BUF[USART2_MAX_RECV_LEN];
extern vu16 USART2_RX_STA;   						//��������״̬
extern u8	GETTING_INFO_FLAG;
extern vu16 BLUETOOTH_MESSAGE_LEN;

void usart2_init(u32 bound);				//����2��ʼ�� 
void u2_printf(char* fmt,...);			//�򴮿ڴ�ӡ����
void init_HC05(void);							//��ʼ��HC-05
u8 get_bluetooth_info(void);						//�����ѯ��Χ������Ϣ
u8 undo_get_bluetooth_info(void);				//ȡ������

#endif













