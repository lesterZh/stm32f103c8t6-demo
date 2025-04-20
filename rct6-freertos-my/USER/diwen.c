#include "diwen.h"

void printU16(u16 val) {
    u8 high = (val >> 8) & 0xFF;
    u8 low = (val) & 0xFF;
    printf("%02X %02X", high, low);
}

void parseDiwenOneWord(u8* buf, int len, u16 *addr, u16 *val) {
    static u16 addr_ = 0, val_ = 0;
    if (len < 8) return;
    if (buf[0] == 0x5A && buf[1] == 0xA5) {
        if (buf[3] == 0x83) {
            addr_ = ((buf[4] << 8) + buf[5]);
            val_  = ((buf[7] << 8) + buf[8]);
            printf("diwen, addr:");
            printU16(addr_);
            printf(", val:");
            printU16(val_);
            printf("\r\n");
            (*addr) = addr_;
            (*val) = val_;
            // setDiwenOneWord(0x1005, 56);
            // queryDiwenOneWord(0x2345);
        }
    }
}

void setDiwenOneWord(u16 addr, u16 val) {
    char cmd[] = {0x5a, 0xa5, 0x05, 0x82, 0x11, 0x22, 0x05, 0x06, 0};

    cmd[4] = ((addr >> 8) & 0xFF);
    cmd[5] = ((addr) & 0xFF);
    cmd[6] = ((val >> 8) & 0xFF);
    cmd[7] = ((val) & 0xFF);

    sendChars(cmd, 8);
}

void queryDiwenOneWord(u16 addr) {
    char cmd[] =  {0x5a, 0xa5, 0x04, 0x83, 0x11, 0x00, 0x01, 0};
    cmd[4] = ((addr >> 8) & 0xFF);
    cmd[5] = ((addr) & 0xFF);

    sendChars(cmd, 7);
}
