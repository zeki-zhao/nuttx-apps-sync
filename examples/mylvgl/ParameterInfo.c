/**
 * @file ParameterInfo.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2026-02-05
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "ParameterInfo.h"
#include "modbus_slave_instance.h"


// u8 u8ParmEnumName; // 参数名称枚举
// u8 u8ByteCount; // 参数数据字节数
// void* Parameter; // 参数起始地址
// u8 u8Permit; // 参数权限
// u32 u32Min; // 参数最小值
// u32 u32Max; // 参数最大值
// u16 u16Flag; // 参数标志
// u16 u16CommCoefDividend; // 通讯系数分母
// u16 u16CommCoefMultiplicand; // 通讯系数分子

// s32 (*PostSetCallBack)(); // 设置值后回调函数
// s32 (*GetValueCallback)(void* databuf, u8* pByteCount); // 获取参数回调函数
// s32 (*SetValueCallback)(void* databuf, u8 u8ByteCount); // 设置参数回调函数
    

/* 参数名称枚举,自动生成 */
ParameterConfigType stParameterConfig[] =
{
    /* Modbus RTU Master Instance APP参数数据 */
    // {PROTOCOL_MODBUS_RTU_MASTER_HOLDING_REG_DATA0, 2, UNSIGNED_YTPE, &stModbusMasterHoldReg.stAppData.u16Data0, READ_WRITE, 0 ,0xffff, 0, 1, 100, NULL, NULL, NULL},
    // {PROTOCOL_MODBUS_RTU_MASTER_HOLDING_REG_DATA1, 2, UNSIGNED_YTPE, &stModbusMasterHoldReg.stAppData.u32Data1, READ_WRITE, 0 ,0xffffffff, 0, 1, 100, NULL, NULL, NULL},
    // {PROTOCOL_MODBUS_RTU_MASTER_HOLDING_REG_DATA2, 2, UNSIGNED_YTPE, &stModbusMasterHoldReg.stAppData.u16Data2, READ_WRITE, 0 ,0xffff, 0, 1, 100, NULL, NULL, NULL},
    // {PROTOCOL_MODBUS_RTU_MASTER_HOLDING_REG_DATA3, 2, UNSIGNED_YTPE, &stModbusMasterHoldReg.stAppData.u16Data3, READ_WRITE, 0 ,0xffff, 0, 1, 100, NULL, NULL, NULL},
    // {PROTOCOL_MODBUS_RTU_MASTER_HOLDING_REG_DATA4, 2, UNSIGNED_YTPE, &stModbusMasterHoldReg.stAppData.u16Data4, READ_WRITE, 0 ,0xffff, 0, 1, 100, NULL, NULL, NULL},
    
    // {PROTOCOL_MODBUS_RTU_MASTER_INPUT_REG_DATA0, 4, UNSIGNED_YTPE, &stModbusMasterInputReg.stAppData.u32Data0, READ_WRITE, 0 ,0xffffffff, 0, 1, 100, NULL, NULL, NULL},
    // {PROTOCOL_MODBUS_RTU_MASTER_INPUT_REG_DATA1, 2, UNSIGNED_YTPE, &stModbusMasterInputReg.stAppData.u16Data1, READ_WRITE, 0 ,0xffff, 0, 1, 100, NULL, NULL, NULL}, 
    // {PROTOCOL_MODBUS_RTU_MASTER_INPUT_REG_DATA2, 2, UNSIGNED_YTPE, &stModbusMasterInputReg.stAppData.u16Data2, READ_WRITE, 0 ,0xffff, 0, 1, 100, NULL, NULL, NULL},
    // {PROTOCOL_MODBUS_RTU_MASTER_INPUT_REG_DATA3, 2, UNSIGNED_YTPE, &stModbusMasterInputReg.stAppData.u16Data3, READ_WRITE, 0 ,0xffff, 0, 1, 100, NULL, NULL, NULL},
    // {PROTOCOL_MODBUS_RTU_MASTER_INPUT_REG_DATA4, 2, UNSIGNED_YTPE, &stModbusMasterInputReg.stAppData.u16Data4, READ_WRITE, 0 ,0xffff, 0, 1, 100, NULL, NULL, NULL},

    /* Modbus RTU Slave Instance APP参数数据 */
    {PROTOCOL_MODBUS_RTU_SlAVE_HOLDING_REGISTERS_DATA6, 4, UNSIGNED_YTPE, &stModbusSlaveHoldReg.stAppData.u32Data6, READ_WRITE, 0 ,0xffffffff, 0, 1, 100, NULL, NULL, NULL},
    {PROTOCOL_MODBUS_RTU_SlAVE_HOLDING_REGISTERS_DATA7, 2, UNSIGNED_YTPE, &stModbusSlaveHoldReg.stAppData.u16Data7, READ_WRITE, 0 ,0xffff, 0, 1, 100, NULL, NULL, NULL},

    {PROTOCOL_MODBUS_RTU_SlAVE_INPUT_REGISTERS_DATA0, 4, UNSIGNED_YTPE, &stModbusSlaveInputReg.stAppData.u32Data0, READ_WRITE, 0 ,0xffffffff, 0, 1, 100, NULL, NULL, NULL},
    {PROTOCOL_MODBUS_RTU_SlAVE_INPUT_REGISTERS_DATA1, 4, UNSIGNED_YTPE, &stModbusSlaveInputReg.stAppData.u32Data1, READ_WRITE, 0 ,0xffffffff, 0, 1, 100, NULL, NULL, NULL},
    {PROTOCOL_MODBUS_RTU_SlAVE_INPUT_REGISTERS_DATA2, 2, UNSIGNED_YTPE, &stModbusSlaveInputReg.stAppData.u16Data2, READ_WRITE, 0 ,0xffff, 0, 1, 100, NULL, NULL, NULL},
    {PROTOCOL_MODBUS_RTU_SlAVE_INPUT_REGISTERS_DATA3, 2, UNSIGNED_YTPE, &stModbusSlaveInputReg.stAppData.u16Data3, READ_WRITE, 0 ,0xffff, 0, 1, 100, NULL, NULL, NULL},
    {PROTOCOL_MODBUS_RTU_SlAVE_INPUT_REGISTERS_DATA6, 4, UNSIGNED_YTPE, &stModbusSlaveInputReg.stAppData.u32Data6, READ_WRITE, 0 ,0xffffffff, 0, 1, 100, NULL, NULL, NULL},
    
};
