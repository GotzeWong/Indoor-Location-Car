#ifndef __SR04_H
#define __SR04_H
#include "sys.h"

#define SR04_TRIG PDout(11)

extern int distence;
extern u8 GET_DIS_FLAG;

void SR04_Init(void);//��ʼ��
int SR04_get_distence(void);//��ȡǰ�����룬��λ����
void send_distence_info(void);//���ͻ�ȡ�ľ���distence��������

#endif
