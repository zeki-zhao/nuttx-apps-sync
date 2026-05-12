#ifndef MODBUS_DATA_H
#define MODBUS_DATA_H

#include <stdint.h>
#include <stddef.h>

#define MODBUS_REG_COUNT 10
#define MODBUS_VALUE_STR_MAX 16

typedef struct {
    uint16_t addr;
    int32_t  value;
    const char *unit;
} modbus_reg_t;

extern modbus_reg_t g_modbus_regs[MODBUS_REG_COUNT];

void modbus_data_init(void);
void modbus_data_set_value(int index, int32_t value);
int32_t modbus_data_get_value(int index);
const char *modbus_data_format_value(int index, char *buf, size_t size);
void modbus_data_simulate(void);

#endif /* MODBUS_DATA_H */
