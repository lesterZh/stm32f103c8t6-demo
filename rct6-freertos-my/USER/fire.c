#include "fire.h"

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef int8_t    s8;
typedef int16_t   s16;
typedef int32_t   s32;

void sys_delay_ms(int ms) {
    vTaskDelay(50);
}

void sys_write_vp(s16 addr, u8 *buf, s16 len) {
}

// ��ǰ���״̬��¼
// 11 ��һ��δ���12����У�13���ɹ���14���ʧ��
// 51 ��5 ��δ���52����У�53���ɹ���54���ʧ��
idata s16 fire_status = 11;

// ����ָ�����
idata u16 cmd_send_cnt;

// ����ʱ 600 s
idata s16 count_down_seconds;

idata s16 btn_type_reg_val = 0;
idata s16 enter_btn_reg_val = 0;

// ��֤���ݽ�����u16 s16
void setDiwenOneWord2(u16 addr, u16 val) {
    u16 v = val;
    sys_write_vp(addr, (u8*) &v, 1);
}

// 100ms����һ��
void read_reg_val() {
    // sys_read_vp(BTN_PRESS_TYPE_REG, (u8*) &btn_type_reg_val, 1);
    // sys_read_vp(BTN_ENTER_PRESS_REG, (u8*) &enter_btn_reg_val, 1);
}


// 300ms ����һ��
void update_cnt_UI() {

    if (count_down_seconds >= 0) {
        setDiwenOneWord(CNT_DOWN_REG, count_down_seconds);
    }

    if (cmd_send_cnt >= 0) {
        setDiwenOneWord(CMD_SEND_CNT_REG, cmd_send_cnt);
    }
}

void enable_all_fire_btn_touch(void);

// ÿ���ӵ���һ��
void count_down_seconds_decrease() {
    count_down_seconds--;

    if (count_down_seconds == 1) {
        add_delay_task(enable_all_fire_btn_touch, 1000);
    }

    if (count_down_seconds <= 0) {
        count_down_seconds = 0;
    }
}

void log_print_chars(char *str, int len) {
    // ���������print�����_DEBUG_ZHT_ �ر��ˣ��������ܷɣ���������ҲҪһ�����뵽��������
    int i=0;
    printf("%d, 2hex : ", len);
    for (; i<len; i++) {
        printf("%02X ", str[i] & 0xFF);
    }
    printf("\r\n");
}


#define CTL_touch_addr 0x00B0
u8 ctl_cmd[8];
idata u16 ctl_touch_delay = 20;

void init_ctl_cmd(void) {
    ctl_cmd[0] = 0x5A;
    ctl_cmd[1] = 0xA5;
    ctl_cmd[2] = 0x00;
    ctl_cmd[3] = 0x00;

    ctl_cmd[4] = 0x01;
    ctl_cmd[5] = 0x05;
    ctl_cmd[6] = 0x00;
    ctl_cmd[7] = 0x00;
}

// 5A A5 0B 82 00 B0 5A A5 00 00 01 05 00 00
uint8_t ctl_cmd2[] = {0x5A, 0xA5, 0x0B, 0x82, 0x00, 0xB0, 
                      0x5A, 0xA5, 0x00, 0x00, 0x01, 0x05, 0x00, 0x00};

void disable_btn_touch2(int id) {
    ctl_cmd2[10] = ((id-1) & 0xFF);
    ctl_cmd2[13] = 0x00;

    uart2SendChars(ctl_cmd2, 14);
    sys_delay_ms(50);
}

void enable_btn_touch2(int id) {
    ctl_cmd2[10] = ((id-1) & 0xFF);
    ctl_cmd2[13] = 0x01;
    uart2SendChars(ctl_cmd2, 14);
    sys_delay_ms(50);
}

//����ָ�� 5A A5 0B 82 00 B0 5A A5 00 00 01 05 00 01
void disable_btn_touch(int id) {
    // [5] 0x05��ʾ���ð�������
    init_ctl_cmd();

    if (id == 1) {
        ctl_cmd[4] = 0x00; 
    } else if (id == 2) {
        ctl_cmd[4] = 0x01; 
    } else if (id == 3) {
        ctl_cmd[4] = 0x02; 
    } else if (id == 4) {
        ctl_cmd[4] = 0x03; 
    } else if (id == 5) {
        ctl_cmd[4] = 0x04; 
    }
    ctl_cmd[4] = ((id-1) & 0xFF);
    printf("dis btn id:%d - ", id);

    // print_hex_array(ctl_cmd, 8);
    // sys_write_vp(CTL_touch_addr, ctl_cmd, 4);

    // �������ʱҪ�㹻�����򲿷ֲ���ʧЧ
    sys_delay_ms(50);
}

void enable_btn_touch(int id) {
    init_ctl_cmd();

    if (id == 1) {
        ctl_cmd[4] = 0x00; 
    } else if (id == 2) {
        ctl_cmd[4] = 0x01; 
    } else if (id == 3) {
        ctl_cmd[4] = 0x02; 
    } else if (id == 4) {
        ctl_cmd[4] = 0x03; 
    } else if (id == 5) {
        ctl_cmd[4] = 0x04; 
    }
    ctl_cmd[4] = ((id-1) & 0xFF);
    ctl_cmd[7] = 0x01;

    printf("enable btn id:%d - ", id);


    // sys_write_vp(CTL_touch_addr, ctl_cmd, 4);
    // �������ʱҪ�㹻�����򲿷ֲ���ʧЧ
    sys_delay_ms(50);
}


typedef void (*btn_ctl_fun)(void);

btn_ctl_fun enable_btn_funs[6];
btn_ctl_fun disable_btn_funs[6];

void init_ctl_btn_touch_fun() {
    init_ctl_cmd();
}

void set_disable_fire_btn_touch_ui(u8 level, s16 val_1_on) {
    setDiwenOneWord(BTN_CTL_TOUCH_BASE_REG + (level),  val_1_on);
}

void disable_fire_btn_touch_exlude(u8 level) {
    // 1������Ӧid=0��5����Ӧid=4
    u8 i;
    u8 id = level;
    int levelInt = level;

    printf("disable_fire_btn_touch_exlude %d\r\n", levelInt);
    
    for (i=1; i<=5; i++) {
        if (1 || i != id) {
            disable_btn_touch2(i);
            // disable_btn_funs[i]();
            // print_hex_array(ctl_cmd, 8);
        }
    }

    for (i=1; i<=5; i++) {
        if (1 || i != id) {
            set_disable_fire_btn_touch_ui(i, 1);
        }
    }
}

void disable_all_fire_btn_touch() {
    // 1������Ӧid=0��5����Ӧid=4
    u8 i;

    printf("disable_all_fire_btn_touch \r\n");
    
    for (i=1; i<=5; i++) {
        disable_btn_touch2(i);
        set_disable_fire_btn_touch_ui(i, 1);
    }
}

void enable_all_fire_btn_touch() {
    // 1������Ӧid=0��5����Ӧid=4
    u8 i = 0;
    printf("enable_all_fire_btn_touch\r\n");
    
    for (i=1; i<=5; i++) {
        enable_btn_touch2(i);
        // enable_btn_funs[i]();
        // print_hex_array(ctl_cmd, 8);
    }

    for (i=1; i<=5; i++) {
        set_disable_fire_btn_touch_ui(i, 0);
    }
}

void start_count_down_seconds() {
    // count_down_seconds = 600;
    count_down_seconds = 10;
}

void set_cmd_wave_ui(int level) {
    // if (level = 1;
    s16 val = level;

    printf("set_cmd_wave_ui %d\r\n", level);
    setDiwenOneWord(CMD_WAVE_REG, val);
}

u8 fire_level;
idata u8 send_fire_cmd_flag = 0;

void send_actual_cmd_to_device(u8 cmd) {
    // u2_send_byte(cmd);
    // u2_send_byte(cmd);
    // u2_send_byte(cmd);
}

void send_start_fire_cmd(int level) {
    cmd_send_cnt++;

    // fire_level = level;
    if (level == 1) {
        send_actual_cmd_to_device(0xB1);
    } else if (level == 2) {
        send_actual_cmd_to_device(0xB2);
    } else if (level == 3) {
        send_actual_cmd_to_device(0xB3);
    } else if (level == 4) {
        send_actual_cmd_to_device(0xB4);
    } else if (level == 5) {
        send_actual_cmd_to_device(0xB5);
    }

    // print_newline();
    // printf("send fire cmd %d\r\n", level);
}

// ÿN���ӵ���һ��
// ����100ms��ѭ������
idata u16 cnt_100ms = 0;
void send_start_fire_cmd_in_loop() {
    if (send_fire_cmd_flag) {
        if (cnt_100ms % 20 == 0) {
            send_start_fire_cmd(fire_level);
        }
        cnt_100ms++;
    }
}

void start_send_fire_cmd(u8 level) {
    send_fire_cmd_flag = 1;
    cnt_100ms = 0;
    fire_level = level;
    set_cmd_wave_ui(level);

    // ָ��ͼ�����ÿ��������
    // cmd_send_cnt = 0;
}

void stop_send_fire_cmd() {
    send_fire_cmd_flag = 0;
    cnt_100ms = 0;
    set_cmd_wave_ui(0);
}

// ��ȡ��������
s16 read_btn_type() {
    static s16 pre_type = 0;
    s16 btn_type = btn_type_reg_val;
    // sys_read_vp(BTN_PRESS_TYPE_REG, (u8*) &btn_type, 1);
    if (pre_type != btn_type) {
        printf("btn type --> %d\r\n", btn_type);
    }
    pre_type = btn_type;
    return btn_type;
}

void reset_btn_type() {
    s16 btn_type = 0;
    setDiwenOneWord(BTN_PRESS_TYPE_REG,  btn_type);
    btn_type_reg_val = 0;
}

u8 isEnterBtnPressed() {
    s16 val = enter_btn_reg_val;
    // sys_read_vp(BTN_ENTER_PRESS_REG, (u8*) &val, 1);
    return val == 1;
}

void setEnterBtnHighLight() {
    s16 val = 1;
    setDiwenOneWord(BTN_ENTER_UI_STATU_REG, val);
}

void resetEnterBtnHighLight() {
    s16 val = 0;
    setDiwenOneWord(BTN_ENTER_UI_STATU_REG,  val);
}

void resetEnterBtnStatus() {
    s16 val = 0;
    setDiwenOneWord(BTN_ENTER_PRESS_REG, val );
    resetEnterBtnHighLight();
    enter_btn_reg_val = 0;
}



void set_5_fire_status(u8 level, s16 status) {
    // ״̬����ֵ�� 0��Ĭ��δ���1������У� 2�����ɹ��� 3�����ʧ��
    s16 val = status;
    setDiwenOneWord(FIRE_1_STATE_REG + (level-1),  val );

    if (status == 1) {
        // ���Ͷ�Ӧ��ָ��
        start_send_fire_cmd(level);

        disable_fire_btn_touch_exlude(level);
    } else if (status == 2) {
        // �ɹ�
        stop_send_fire_cmd();
        start_count_down_seconds();

        disable_fire_btn_touch_exlude(level);
    } else if (status == 3) {
        // ʧ��
        stop_send_fire_cmd();

        enable_all_fire_btn_touch();
    }
}

void reset_all_fire_status() {
    u8 i=1;
    for (; i<=5; i++) {
        set_5_fire_status(i, FIRE_NOT);
    }
    fire_status = 11;
    count_down_seconds = 0;
    cmd_send_cnt = 0;
    stop_send_fire_cmd();

    enable_all_fire_btn_touch();
}

s16 key_code_cur;

void delay_fun_reset_enter_btn(void) {
    resetEnterBtnHighLight();
    // ����Ҫ����Ĳ��� 
    // resetEnterBtnStatus();

    // ֻ���� enter ��������ʱ����һ�ΰ���ֵ����Ч������֮����һ�ΰ���ֵ�������
    // ��ֹ�����
    key_code_cur = 0;
}

void delay_fun_hide_warn_info(void) {
    s16 val = 0;
    setDiwenOneWord(WARNING_TIP_REG, val);
    // printf("hide warning\r\n");
}

void show_warn_info() {
    s16 val = 1;
    setDiwenOneWord(WARNING_TIP_REG, val);
    add_delay_task(delay_fun_hide_warn_info, 1000);
    // printf("show warning\r\n");
}

#define FIRE_START_CHECK if (count_down_seconds > 0 || fire_status % 10 == 2) { show_warn_info(); printf("fire status now:%d, forbid\r\n", fire_status); return; }            

// 100ms ����һ��
// ����ָʾֵ��1���������Ϊ1��5�����5�� ����6���ɹ�7��ʧ��8��
void key_process() {
    u8 level;
    s16 key_code = read_btn_type();
    if (key_code >= 1 && key_code <= 8) {
        reset_btn_type();
        setEnterBtnHighLight();

        add_delay_task_for_enter_btn_change(delay_fun_reset_enter_btn, 2000);

        key_code_cur = key_code;
    }

    if (isEnterBtnPressed()) {
        resetEnterBtnStatus();

        if (key_code_cur == 1) {
            FIRE_START_CHECK;
            fire_status = 12;
            set_5_fire_status(1, FIRE_ING);
        } else if (key_code_cur == 2) {
            // �����ǰ���ڵ���ʱ�����ߴ��ڵ��״̬������Ӧ
            FIRE_START_CHECK;
            fire_status = 22;
            set_5_fire_status(2, FIRE_ING);
        } else if (key_code_cur == 3) {
            FIRE_START_CHECK;
            fire_status = 32;
            set_5_fire_status(3, FIRE_ING);
        } else if (key_code_cur == 4) {
            FIRE_START_CHECK;
            fire_status = 42;
            set_5_fire_status(4, FIRE_ING);
        } else if (key_code_cur == 5) {
            FIRE_START_CHECK;
            fire_status = 52;
            set_5_fire_status(5, FIRE_ING);
        } else if (key_code_cur == 7) {
            // if (fire_status % 10 != 2) 
            fire_status = fire_status + 1;
            level = fire_status / 10;
            set_5_fire_status(level, FIRE_OK);
        } else if (key_code_cur == 8) {
            fire_status = fire_status + 2;
            level = fire_status / 10;
            set_5_fire_status(level, FIRE_FAIL);
        } else if (key_code_cur == 6) {
            reset_all_fire_status();
        } 

        key_code_cur = 0;
    }
}

void fire_task(void *arg)
{
    while (1)
    {
        key_process();
        send_start_fire_cmd_in_loop();
        delay_task_scheduler();
        vTaskDelay(100);
    }
}

void fire_task_ui(void *arg)
{
    int cnt = 0;
    while (1)
    {
        update_cnt_UI();
        if (cnt % 2 == 1) {
            count_down_seconds_decrease();
        }
        vTaskDelay(500);
        cnt++;
    }
}

void fire_task_init() {
    init_ctl_cmd();
    init_delay_task();

    xTaskCreate(fire_task, "fire_task", 128, NULL, 2, NULL);
    xTaskCreate(fire_task_ui, "fire_task_ui", 128, NULL, 2, NULL);
}




