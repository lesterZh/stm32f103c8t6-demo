#include "delay.h"




//��ʼ���ӳٺ���
//��ʹ��OS��ʱ��,�˺������ʼ��OS��ʱ�ӽ���
//���õȴ�ʱ�䵥λΪ1ms
void delay_init(void)
{
	if(SysTick_Config(SystemCoreClock / 1000))//systick�ж����ã�1ms�����ж�һ�Σ�stm32f10x_it.c �ļ��е�SysTick_Handler�����е��ü���
  { 	
		
  }
}								    


static __IO uint32_t TimingDelay;//ʱ�����ֵ
//1ms��ʱ
void delay_ms(__IO uint32_t  nTime)
{	 		  	  
  TimingDelay = nTime;

  while(TimingDelay != 0);
 	    
} 

//1ms������һ��
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}













