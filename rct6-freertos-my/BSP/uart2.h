#ifndef __USAR2_H
#define __USAR2_H

#include "stdio.h"	
#include "sys.h" 

void uart2_init(int bound);
void uart2SendChars(char chs[], int len);
void uart2SendString(char *str);


void resetUart2RecBuf(void);
u8 * getUart2RecBuf(void);
void set_uart2_rec_tick(void);
int isUart2RecFrame(void);
int getUart2RecLen(void);

void test_uart2_dma_send(void);
void test_dma_rec(void);

#endif

