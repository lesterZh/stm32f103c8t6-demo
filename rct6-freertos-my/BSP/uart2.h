#ifndef __USAR2_H
#define __USAR2_H

#include "stdio.h"	
#include "sys.h" 

void uart2_init(int bound);
void uart2SendChars(char chs[], int len);
void uart2SendString(char *str);


void resetUart2RecBuf();
u8 * getUart2RecBuf();
void set_uart2_rec_tick();
int isUart2RecFrame();
int getUart2RecLen();

void test_uart2_dma_send();
void test_dma_rec();

#endif