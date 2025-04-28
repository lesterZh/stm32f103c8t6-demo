#ifndef __FIRE_H__
#define __FIRE_H__

#include "sys.h"
#include "delay_task.h"
// #include "uart2.h"
// #include "T5L_LIB.h"


#include "diwen.h"
#include "freertos_program.h"
#define idata volatile

// 状态变量值， 0：默认未点火，1：点火中， 2：点火成功， 3：点火失败
// 一级点火状态指示变量，控制点火的图标显示

#define FIRE_1_STATE_REG 0x1201
#define FIRE_2_STATE_REG 0x1202
#define FIRE_3_STATE_REG 0x1203
#define FIRE_4_STATE_REG 0x1204
#define FIRE_5_STATE_REG 0x1205

#define FIRE_NOT 0
#define FIRE_ING 1
#define FIRE_OK  2
#define FIRE_FAIL  3

// 按键指示值，1级点火设置为1；5级点火5； 重置6，成功7，失败8；
#define BTN_PRESS_TYPE_REG 0x1210

// 确认按钮，按下为1，默认0，确认事件处理之后，按键变量都变成0
#define BTN_ENTER_PRESS_REG 0x1212

// 确认按钮状态指示，1为提示状态，0为默认
#define BTN_ENTER_UI_STATU_REG 0x1213

// 负载状态
#define PAYLOAD_STATE_REG 0x1216

// 波形显示
#define CMD_WAVE_REG 0x1218

// 命令发送次数
#define CMD_SEND_CNT_REG 0x1220

// 倒计时
#define CNT_DOWN_REG 0x1226

// 警告提示
#define WARNING_TIP_REG 0x1228

// 触摸禁用图标控制 0x1231 ~ 0x1235
#define BTN_CTL_TOUCH_BASE_REG 0x1230


#define CNT_DOWN_VAL 10

extern idata u16 cmd_send_cnt;
extern idata s16 count_down_seconds;
extern idata s16 btn_type_reg_val;
extern idata s16 enter_btn_reg_val;
extern idata s16 payload_btn_reg_val;

void read_reg_val();
void send_start_fire_cmd_in_loop();
void update_cnt_UI();
void count_down_seconds_decrease();

void key_process();

void init_ctl_btn_touch_fun();

void fire_task_init();
#endif

