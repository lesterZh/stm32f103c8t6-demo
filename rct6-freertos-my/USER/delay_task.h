#ifndef __DELAY_TASK_H__
#define __DELAY_TASK_H__
#include "sys.h"
#include "stdio.h"

#define idata volatile
#define MAX_TASKS 50

typedef struct {
    u8 active;
    u32 start_time;
    u32 delay;
    void (*callback)(void);
} DelayTask;

void sys_handler_delay_task();
void init_delay_task(void);
void add_delay_task(void (*callback)(void), u32 delay_ms);
void delay_task_scheduler(void);
void add_delay_task_for_enter_btn_change(void (*callback)(void), u32 delay_ms) ;

#endif