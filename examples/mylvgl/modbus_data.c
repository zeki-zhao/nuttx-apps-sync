#include "modbus_data.h"
#include <stdlib.h>
#include <stdio.h>

modbus_reg_t g_modbus_regs[MODBUS_REG_COUNT];

void modbus_data_init(void)
{
    g_modbus_regs[0]  = (modbus_reg_t){0x0000, 25,   "°C"};
    g_modbus_regs[1]  = (modbus_reg_t){0x0001, 50,   "%RH"};
    g_modbus_regs[2]  = (modbus_reg_t){0x0002, 1013, "hPa"};
    g_modbus_regs[3]  = (modbus_reg_t){0x0003, 220,  "V"};
    g_modbus_regs[4]  = (modbus_reg_t){0x0004, 5,    "A"};
    g_modbus_regs[5]  = (modbus_reg_t){0x0005, 1500, "RPM"};
    g_modbus_regs[6]  = (modbus_reg_t){0x0006, 37,   "°C"};
    g_modbus_regs[7]  = (modbus_reg_t){0x0007, 75,   "%"};
    g_modbus_regs[8]  = (modbus_reg_t){0x0008, 12,   "mA"};
    g_modbus_regs[9]  = (modbus_reg_t){0x0009, 99,   "%"};
}

void modbus_data_set_value(int index, int32_t value)
{
    if (index >= 0 && index < MODBUS_REG_COUNT)
        g_modbus_regs[index].value = value;
}

int32_t modbus_data_get_value(int index)
{
    if (index >= 0 && index < MODBUS_REG_COUNT)
        return g_modbus_regs[index].value;
    return 0;
}

const char *modbus_data_format_value(int index, char *buf, size_t size)
{
    if (index < 0 || index >= MODBUS_REG_COUNT)
        return "";

    snprintf(buf, size, "%d", g_modbus_regs[index].value);
    return buf;
}

void modbus_data_simulate(void)
{
    for (int i = 0; i < MODBUS_REG_COUNT; i++) {
        int32_t v = g_modbus_regs[i].value;
        int32_t delta = rand() % 5 - 2;

        switch (i) {
        case 0:
            v += delta;
            if (v < 18 || v > 35) v = 25;
            break;
        case 1:
            v += delta;
            if (v < 30 || v > 80) v = 50;
            break;
        case 2:
            v += delta;
            if (v < 990 || v > 1030) v = 1013;
            break;
        case 3:
            v += delta;
            if (v < 210 || v > 240) v = 220;
            break;
        case 4:
            v += delta;
            if (v < 3 || v > 8) v = 5;
            break;
        case 5:
            v += delta * 10;
            if (v < 500 || v > 3000) v = 1500;
            break;
        case 6:
            v += delta;
            if (v < 30 || v > 45) v = 37;
            break;
        case 7:
            v += delta;
            if (v < 0 || v > 100) v = 75;
            break;
        case 8:
            v += delta;
            if (v < 4 || v > 20) v = 12;
            break;
        case 9:
            v += delta;
            if (v < 90 || v > 100) v = 99;
            break;
        default:
            break;
        }

        g_modbus_regs[i].value = v;
    }
}
