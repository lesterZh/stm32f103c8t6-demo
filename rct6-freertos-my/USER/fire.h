#ifndef __FIRE_H__
#define __FIRE_H__

#include "sys.h"
#include "delay_task.h"
// #include "uart2.h"
// #include "T5L_LIB.h"


#include "diwen.h"
#include "freertos_program.h"
#define idata volatile

// ״̬����ֵ�� 0��Ĭ��δ���1������У� 2�����ɹ��� 3�����ʧ��
// һ�����״ָ̬ʾ���������Ƶ���ͼ����ʾ

#define FIRE_1_STATE_REG 0x1201
#define FIRE_2_STATE_REG 0x1202
#define FIRE_3_STATE_REG 0x1203
#define FIRE_4_STATE_REG 0x1204
#define FIRE_5_STATE_REG 0x1205

#define FIRE_NOT 0
#define FIRE_ING 1
#define FIRE_OK  2
#define FIRE_FAIL  3

// ����ָʾֵ��1���������Ϊ1��5�����5�� ����6���ɹ�7��ʧ��8��
#define BTN_PRESS_TYPE_REG 0x1210

// ȷ�ϰ�ť������Ϊ1��Ĭ��0��ȷ���¼�����֮�󣬰������������0
#define BTN_ENTER_PRESS_REG 0x1212

// ȷ�ϰ�ť״ָ̬ʾ��1Ϊ��ʾ״̬��0ΪĬ��
#define BTN_ENTER_UI_STATU_REG 0x1213

// ����״̬
#define PAYLOAD_STATE_REG 0x1216

// ������ʾ
#define CMD_WAVE_REG 0x1218

// ����ʹ���
#define CMD_SEND_CNT_REG 0x1220

// ����ʱ
#define CNT_DOWN_REG 0x1226

// ������ʾ
#define WARNING_TIP_REG 0x1228

// ��������ͼ����� 0x1231 ~ 0x1235
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

