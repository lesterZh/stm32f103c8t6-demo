#include "delay_task.h"

DelayTask delay_tasks[MAX_TASKS];
idata u32 sys_tick_delay_task = 0;

void sys_handler_delay_task() {
    sys_tick_delay_task++;
}

// 默认数据是放在xdata区，需要系统初始化后才能使用
void init_delay_task(void) {
    u8 i = 0;
    for ( ; i < MAX_TASKS; i++) {
        delay_tasks[i].active = 0;
        delay_tasks[i].start_time = 0;
        delay_tasks[i].delay = 0;
        delay_tasks[i].callback = NULL;
    }
}

void add_delay_task(void (*callback)(void), u32 delay_ms) {
    int i = 0;
    for ( ; i < MAX_TASKS; i++) {
        if (delay_tasks[i].active == 0) {
            delay_tasks[i].active = 1;
            delay_tasks[i].start_time = sys_tick_delay_task;
            delay_tasks[i].delay = delay_ms;
            delay_tasks[i].callback = callback;
            printf("add delay task : %d\r\n", i);
            break;
        }
    }
}

void add_delay_task_for_enter_btn_change(void (*callback)(void), u32 delay_ms) {
    int i = 0;
    delay_tasks[i].active = 1;
    delay_tasks[i].start_time = sys_tick_delay_task;
    delay_tasks[i].delay = delay_ms;
    delay_tasks[i].callback = callback;
    printf("add delay task for enter btn : %d\r\n", i);
}

void delay_task_scheduler(void) {
    int i = 0;
    for ( ; i < MAX_TASKS; i++) {
        if (delay_tasks[i].active &&
            (sys_tick_delay_task - delay_tasks[i].start_time >= delay_tasks[i].delay)) {

            if (delay_tasks[i].callback != NULL) {
                delay_tasks[i].callback();
            }
            
            delay_tasks[i].active = 0;
            delay_tasks[i].delay = 0;
            delay_tasks[i].callback = NULL;
            printf("do  delay task : %d -->>\r\n", i);
        }
    }
}
