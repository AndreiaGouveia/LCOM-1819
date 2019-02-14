#include "rtc.h"
#include <macro.h>
#include "video.h"
#include "include.h"

int hook_id_rtc = 8;

int subscribe_rtc()
{
    hook_id_rtc = RTC_IRQ;

    return sys_irqsetpolicy(RTC_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id_rtc);
}

int unsubscribe_rtc()
{
    return sys_irqrmpolicy(&hook_id_rtc);
}

int convert(uint8_t number)
{
    int r;
    uint8_t temp = number & 0x0F;

    switch (temp)
    {
    case 0x0A:
        temp = 10;
        break;
    case 0x0B:
        temp = 10;
        break;
    case 0x0C:
        temp = 10;
        break;
    case 0x0D:
        temp = 10;
        break;
    case 0x0E:
        temp = 10;
        break;
    case 0x0F:
        temp = 10;
        break;
    }

    r = (int)temp;
    temp = (number >> 4) & 0x0F;

    switch (temp)
    {
    case 0x0A:
        temp = 10;
        break;
    case 0x0B:
        temp = 11;
        break;
    case 0x0C:
        temp = 12;
        break;
    case 0x0D:
        temp = 13;
        break;
    case 0x0E:
        temp = 14;
        break;
    case 0x0F:
        temp = 15;
        break;
    }

    r += (int)(temp * 10);
    return r;
}

int read_reg(uint8_t reg, uint8_t *command)
{

    if (sys_outb(RTC_ADRR_REG, reg) != OK)
        return 1;

    uint32_t data;

    if (sys_inb(RTC_DATA_REG, &data) != OK)
        return 1;

    *command = (uint8_t)data;
    return 0;
}

int seconds()
{
    uint8_t data;

    if (read_reg(SECONDS, &data) != OK)
        return RTC_FAIL;

    return convert(data);
}

int minutes()
{
    uint8_t data;

    if (read_reg(MINUTES, &data) != OK)
        return RTC_FAIL;

    return convert(data);
}

int hour()
{
    uint8_t data;

    if (read_reg(HOURS, &data) != OK)
        return RTC_FAIL;

    return convert(data);
}

int day()
{
    uint8_t data;

    if (read_reg(DATE_OF_MONTH, &data) != OK)
        return RTC_FAIL;

    return convert(data);
}

int month()
{
    uint8_t data;

    if (read_reg(MONTH, &data) != OK)
        return RTC_FAIL;

    return convert(data);
}

int year()
{
    uint8_t data;

    if (read_reg(YEAR, &data) != OK)
        return RTC_FAIL;

    return convert(data);
}

void update_hour()
{
    date.year = year() + 2000;

    date.month = month();

    date.day = day();

    date.seconds = seconds();

    date.minutes = minutes();

    date.hour = hour();
}

void printDate()
{
    int x = 900;
    update_hour();
   // int unidadesSegundo = date.seconds % 10;
   // int dezenasSegundo = date.seconds / 10;
    int unidadesMinuto = date.minutes % 10;
    int dezenasMinuto = date.minutes / 10;
    int unidadesDia = date.day % 10;
    int dezenasDia = date.day / 10;
    int unidadesMes = date.month % 10;
    int dezenasMes = date.month / 10;
    /*int unidadesAno = date.year % 10;
    int dezenasAno = unidadesAno % 10;
    int centenasAno = dezenasAno % 10;
    int milharesAno = centenasAno / 10;*/
    int unidadesHora = date.hour % 10;
    int dezenasHora = date.hour / 10;

    draw_rectangle(850, 0, 174, 50, 0xFFFFFF);

    IntToDraw(dezenasDia, x, 2);
    x += 10;
    IntToDraw(unidadesDia, x, 2);
    x += 15;

    IntToDraw(dezenasMes, x, 2);
    x += 10;
    IntToDraw(unidadesMes, x, 2);
    x += 15;

    IntToDraw(2, x, 2);
    x += 10;
    IntToDraw(0, x, 2);
    x += 10;
    int year1=year();
    year1=year1%10;
    IntToDraw(1, x, 2);
    x += 10;
    IntToDraw(year1, x, 2);
    x = 900;

    IntToDraw(dezenasHora, x, 25);
    x += 10;
    IntToDraw(unidadesHora, x, 25);
    x += 10;

    xpm_image_t xpm_imageT;
    xpm_load(TWODOTS_xpm, XPM_8_8_8, &xpm_imageT);
    DrawMap((char *)xpm_imageT.bytes, 10, 20, x, 25);
    x+=10;
    IntToDraw(dezenasMinuto, x, 25);
    x += 10;
    IntToDraw(unidadesMinuto, x, 25);
    x += 10;
/*
    IntToDraw(dezenasSegundo, x, 25);
    x += 10;
    IntToDraw(unidadesSegundo, x, 25);
    x += 10;
*/
    backround_to_videomem();
}
