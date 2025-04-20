#ifndef __DIWEN_H__
#define __DIWEN_H__

#include "stm32f10x.h" // Device header
#include "usart.h"
#include "sys.h"

void parseDiwenOneWord(u8* buf, int len, u16 *addr, u16 *val);
void setDiwenOneWord(u16 addr, u16 val);
void queryDiwenOneWord(u16 addr) ;

#endif