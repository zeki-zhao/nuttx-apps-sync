#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <poll.h>
#include <termios.h>
#include "ProtocolManager.h"
#include "ParameterInfo.h"

#include "modbus_slave_instance.h"
#include "modbus_slave.h"

#define MODEBUS_TTY_DEV  "/dev/ttyS1"
#define FRAME_BUF_SIZE   300
#define FRAME_TIMEOUT_MS 20   /* inter-byte timeout for frame end detection */

int g_modbus_fd = -1;

modbus_reg_t g_modbus_regs[MODBUS_REG_COUNT];
ProtocolManagerType stProtocolManager_ModbusRtu_Slave;
static u8 u8SlaveSendbuf[FRAME_BUF_SIZE];
static u8 u8SlaveRecvbuf[FRAME_BUF_SIZE];

/* Frame accumulation state: last-receive timestamp */
static int      g_frame_len;
static uint64_t g_frame_tick;

static uint64_t tick_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
}

void modbus_data_init(void)
{
    g_modbus_fd = open(MODEBUS_TTY_DEV, O_RDWR | O_NONBLOCK);
    if (g_modbus_fd < 0){
        printf("modbus: open /dev/ttyS1 failed: %d\n", errno);
    }

        s32 s32Ret = 0;
    s32Ret = ProtocolManager_Init(&stProtocolManager_ModbusRtu_Slave, u8SlaveSendbuf, 300, u8SlaveRecvbuf, 300, stParameterConfig, ALL_PARAMETER_NUM, NULL);
    if(s32Ret!= 0){
        printf("ProtocolManager_Init failed\n");
    }
    s32Ret = ProtocolManager_HalfDuplexSlaveModeInit(&stProtocolManager_ModbusRtu_Slave);
    if(s32Ret!= 0){
        printf("ProtocolManager_HalfDuplexSlaveModeInit failed\n");
    }
    s32Ret = ProtocolManager_HalfDuplexSlaveProtocolRegister(&stProtocolManager_ModbusRtu_Slave, &stModbusRtu_Slave_Instance);
    if(s32Ret!= 0){
        printf("ProtocolManager_HalfDuplexSlaveProtocolRegister failed\n");
    }

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

    snprintf(buf, size, "%d", stModbusSlaveHoldReg.u16HoldReg[index]);
    return buf;
}


void modbus_serial_poll(void)
{
    if (g_modbus_fd < 0)
        return;
    /* Wait for data or timeout — 20ms doubles as frame-boundary detection */
    struct pollfd fdp;
    fdp.fd     = g_modbus_fd;
    fdp.events = POLLIN;
    if (poll(&fdp, 1, FRAME_TIMEOUT_MS) > 0 && (fdp.revents & POLLIN))
    {
        u8 byte;
        while (read(g_modbus_fd, &byte, 1) > 0)
        {
            if (g_frame_len >= FRAME_BUF_SIZE)
                g_frame_len = 0;
            u8SlaveRecvbuf[g_frame_len++] = byte;
            g_frame_tick = tick_ms();
        }
    }

    /* Frame complete ? — timeout since last byte received */
    if (g_frame_len > 0 && tick_ms() - g_frame_tick >= FRAME_TIMEOUT_MS)
    {
        if (g_frame_len >= 5)
        {
            ProtocolManager_RecvMsgReady(&stProtocolManager_ModbusRtu_Slave,g_frame_len);
        }
        g_frame_len = 0;
    }
}

void modbus_timer_tick(void)
{
    ProtocolManager_Process(&stProtocolManager_ModbusRtu_Slave);
    if(ProtocolManager_TryConsumeSendReadyFlag(&stProtocolManager_ModbusRtu_Slave)){
        write(g_modbus_fd, u8SlaveSendbuf, stProtocolManager_ModbusRtu_Slave.u16SendMsgLength);
    }
}

uint16_t modbus_slave_read_reg(int reg_type, int index)
{
    if (index < 0 || index >= MODBUS_REG_ARRAY_SIZE)
        return 0;
    if (reg_type == MODBUS_REG_TYPE_INPUT)
        return stModbusSlaveInputReg.u16InputReg[index];
    return stModbusSlaveHoldReg.u16HoldReg[index];
}
