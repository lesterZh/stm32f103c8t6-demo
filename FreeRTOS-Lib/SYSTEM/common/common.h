#ifndef __COMMON_H__
#define __COMMON_H__	 
#include "sys.h"
#include "usart.h"		
#include "delay.h"	 	 	 	 	 
#include "string.h"


//�ṹ������������Ϊȫ�ֱ���ʹ�õĻ�����Ҫ��ͷ�ļ���������
struct stuSystemStatus{
	int bLightOpen;
    int bDoorSwitch;
	int bWindowCurtainsOpen;
    float fLightIntensity;
};	

extern struct stuSystemStatus mSysStatus;

#endif





