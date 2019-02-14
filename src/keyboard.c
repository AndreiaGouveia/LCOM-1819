#include <lcom/lcf.h>
#include <keyboard.h>
#include "Mouse.h"
#include <macro.h>
#include <string.h>
#include "include.h"
uint32_t cnt = 0;
int hook_ID = 1;
uint8_t scan_Code;
#ifdef LAB3
int sys_inb_cnt(port_t port, uint32_t *data1)
{
    if (sys_inb(port, data1) != OK)
        return 1;
    cnt++;
    return 0;
}
#else
#define sys_inb_cnt(p, q) sys_inb(p, q)
#endif

void(kbc_ih)()
{
    uint32_t status, data;

    sys_inb_cnt(STAT_REG, &status); //assuming it returns OK
    status = (uint8_t)status;

    sys_inb_cnt(OUT_BUF, &data); //assuming it returns OK
    if ((status & (PAR_ERR | TO_ERR)) == 0)
    {
        data = (uint8_t)data;
        scan_Code = data;
    }
}

int keyboard_subscribe(uint8_t *bit_no)
{
    *bit_no = hook_ID;

    if (sys_irqsetpolicy(KBD_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_ID) != OK) //to disable ih from reading the interuptions
        return 1;

    return 0;
}

int keyboard_unsubscribe()
{
    if (sys_irqrmpolicy(&hook_ID) != OK)
        return 1;
    return 0;
}

bool is_make()
{
    if ((scan_Code & mask_make) != mask_make) //if it is a make code
    {
        return true;
    }
    else
    {
        return false;
    }
}

int ScanToAsc()
{
    switch (scan_Code)
    {
    case /* */ 0x39:
        return 32;
    case /*0*/ 0x0b:
        return 48;
    case /*1*/ 0x02:
        return 49;
    case /*2*/ 0x03:
        return 50;
    case /*3*/ 0x04:
        return 51;
    case /*4*/ 0x05:
        return 52;
    case /*5*/ 0x06:
        return 53;
    case /*6*/ 0x07:
        return 54;
    case /*7*/ 0x08:
        return 55;
    case /*8*/ 0x09:
        return 56;
    case /*9*/ 0x0a:
        return 57;
    case /*A*/ 0x1e:
        return 65;
    case /*B*/ 0x30:
        return 66;
    case /*C*/ 0x2e:
        return 67;
    case /*D*/ 0x20:
        return 68;
    case /*E*/ 0x12:
        return 69;
    case /*F*/ 0x21:
        return 70;
    case /*G*/ 0x22:
        return 71;
    case /*H*/ 0x23:
        return 72;
    case /*I*/ 0x17:
        return 73;
    case /*J*/ 0x24:
        return 74;
    case /*K*/ 0x25:
        return 75;
    case /*L*/ 0x26:
        return 76;
    case /*M*/ 0x32:
        return 77;
    case /*N*/ 0x31:
        return 78;
    case /*O*/ 0x18:
        return 79;
    case /*P*/ 0x19:
        return 80;
    case /*Q*/ 0x10:
        return 81;
    case /*R*/ 0x13:
        return 82;
    case /*S*/ 0x1f:
        return 83;
    case /*T*/ 0x14:
        return 84;
    case /*U*/ 0x16:
        return 85;
    case /*V*/ 0x2f:
        return 86;
    case /*W*/ 0x11:
        return 87;
    case /*X*/ 0x2d:
        return 88;
    case /*Y*/ 0x15:
        return 89;
    case /*Z*/ 0x2c:
        return 90;
    case /*BACKSPACE*/ 0X0e:
        return 8;
    case /*ENTER*/ 0X1c:
        return 11;
    }
    return 0;
}
