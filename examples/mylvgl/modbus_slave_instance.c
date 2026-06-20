/**
 * @file Protocol_ModbusRtu_Slave_Instance.c
 * @author zk (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2026-03-19
 * 
 * @copyright Copyright (c) 2026
 * 
 */
/***********************************************************************************************************************
*                                                     INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
***********************************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ParameterManager.h"
#include "ProtocolManager.h"
#include "ParameterInfo.h"

#include "Protocol_ModbusRtu.h"
#include "modbus_slave_instance.h"


/***********************************************************************************************************************
*                                            SOURCE FILE VERSION INFORMATION
***********************************************************************************************************************/

/***********************************************************************************************************************
*                                                  FILE VERSION CHECKS
***********************************************************************************************************************/

/***********************************************************************************************************************
*                                                   DEFINES AND MACROS
***********************************************************************************************************************/

/***********************************************************************************************************************
*                                                        ENUMS
***********************************************************************************************************************/

/***********************************************************************************************************************
*                                              STRUCTURES AND OTHER TYPEDEFS
***********************************************************************************************************************/

/***********************************************************************************************************************
*                                               STATIC FUNCTION PROTOTYPES
***********************************************************************************************************************/

/***********************************************************************************************************************
*                                              STATIC FUNCTION DEFINITIONS
***********************************************************************************************************************/

/**
 * @brief 异常处理
 * 
 * @param u16ProtocolErrorStatus 故障码
 * @param pPrivate 协议编解码过程私有信息
 */
static void ErrorProcess(HalfDuplexSlaveProtocolInstanceType* pInstance)
{
    // 在此处实现异常处理逻辑
    ProtocolErrorCodeType Temp = (ProtocolErrorCodeType)pInstance->u16ProtocolErrorStatus;
    printf("modbus slave instance error: %d\n", Temp);
}


static s32 HoldRegisters_Table_GetValueCallback_0_2(u16 ParmEnumName, u8* pDataAddr, u32 u32StartIndex, u16 u16ByteNumMax, void* pPrivate)
{
    //小端转大端
    pDataAddr[0] = (u8)((stModbusSlaveHoldReg.u16HoldReg[0] & 0xFF00) >> 8);
    pDataAddr[1] = (u8)(stModbusSlaveHoldReg.u16HoldReg[0] & 0x00FF);
    pDataAddr[2] = (u8)((stModbusSlaveHoldReg.u16HoldReg[1] & 0xFF00) >> 8);
    pDataAddr[3] = (u8)(stModbusSlaveHoldReg.u16HoldReg[1] & 0x00FF);
    pDataAddr[4] = (u8)((stModbusSlaveHoldReg.u16HoldReg[2] & 0xFF00) >> 8);
    pDataAddr[5] = (u8)(stModbusSlaveHoldReg.u16HoldReg[2] & 0x00FF);
    return 0;
}
static s32 HoldRegisters_Table_SetValueCallback_0_2(u16 ParmEnumName, const u8* pDataAddr, u32 u32StartIndex, u16 u16ByteNumMax, void* pPrivate)
{
    stModbusSlaveHoldReg.u16HoldReg[0] = ((u16)pDataAddr[0]<<8) | (u16)pDataAddr[1];
    stModbusSlaveHoldReg.u16HoldReg[1] = ((u16)pDataAddr[2]<<8) | (u16)pDataAddr[3];
    stModbusSlaveHoldReg.u16HoldReg[2] = ((u16)pDataAddr[4]<<8) | (u16)pDataAddr[5];
    return 0;
}

static s32 HoldRegisters_Table_GetValueCallback_3_3(u16 ParmEnumName, u8* pDataAddr, u32 u32StartIndex, u16 u16ByteNumMax, void* pPrivate)
{
    pDataAddr[0] = (u8)((stModbusSlaveHoldReg.u16HoldReg[3] & 0xFF00) >> 8);
    pDataAddr[1] = (u8)(stModbusSlaveHoldReg.u16HoldReg[3] & 0x00FF);
    return 0;
}
static s32 HoldRegisters_Table_SetValueCallback_3_3(u16 ParmEnumName, const u8* pDataAddr, u32 u32StartIndex, u16 u16ByteNumMax, void* pPrivate)
{
    stModbusSlaveHoldReg.u16HoldReg[3] = ((u16)pDataAddr[0]<<8) | (u16)pDataAddr[1];
    return 0;
}

static s32 HoldRegisters_Table_GetValueCallback_4_4(u16 ParmEnumName, u8* pDataAddr, u32 u32StartIndex, u16 u16ByteNumMax, void* pPrivate)
{
    pDataAddr[0] = (u8)((stModbusSlaveHoldReg.u16HoldReg[4] & 0xFF00) >> 8);
    pDataAddr[1] = (u8)(stModbusSlaveHoldReg.u16HoldReg[4] & 0x00FF);
    return 0;
}
static s32 HoldRegisters_Table_SetValueCallback_4_4(u16 ParmEnumName, const u8* pDataAddr, u32 u32StartIndex, u16 u16ByteNumMax, void* pPrivate)
{
    stModbusSlaveHoldReg.u16HoldReg[4] = ((u16)pDataAddr[0]<<8) | (u16)pDataAddr[1];
    return 0;
}

static s32 HoldRegisters_Table_GetValueCallback_5_6(u16 ParmEnumName, u8* pDataAddr, u32 u32StartIndex, u16 u16ByteNumMax,void* pPrivate)
{
    pDataAddr[0] = (u8)((stModbusSlaveHoldReg.u16HoldReg[5] & 0xFF00) >> 8);
    pDataAddr[1] = (u8)(stModbusSlaveHoldReg.u16HoldReg[5] & 0x00FF);
    pDataAddr[2] = (u8)((stModbusSlaveHoldReg.u16HoldReg[6] & 0xFF00) >> 8);
    pDataAddr[3] = (u8)(stModbusSlaveHoldReg.u16HoldReg[6] & 0x00FF);
    return 0;
}
static s32 HoldRegisters_Table_SetValueCallback_5_6(u16 ParmEnumName, const u8* pDataAddr, u32 u32StartIndex, u16 u16ByteNumMax, void* pPrivate)
{
    stModbusSlaveHoldReg.u16HoldReg[5] = ((u16)pDataAddr[0]<<8) | (u16)pDataAddr[1];
    stModbusSlaveHoldReg.u16HoldReg[6] = ((u16)pDataAddr[2]<<8) | (u16)pDataAddr[3];
    return 0;
}


static s32 HoldRegisters_Table_GetValueCallback_7_7(u16 ParmEnumName, u8* pDataAddr, u32 u32StartIndex, u16 u16ByteNumMax, void* pPrivate)
{
    pDataAddr[0] = (u8)((stModbusSlaveHoldReg.u16HoldReg[7] & 0xFF00) >> 8);
    pDataAddr[1] = (u8)(stModbusSlaveHoldReg.u16HoldReg[7] & 0x00FF);
    return 0;    
}
static s32 HoldRegisters_Table_SetValueCallback_7_7(u16 ParmEnumName, const u8* pDataAddr, u32 u32StartIndex, u16 u16ByteNumMax, void* pPrivate)
{
    stModbusSlaveHoldReg.u16HoldReg[7] = ((u16)pDataAddr[0]<<8) | (u16)pDataAddr[1];
    return 0;
}

static s32 HoldRegisters_Table_GetValueCallback_8_11(u16 ParmEnumName, u8* pDataAddr, u32 u32StartIndex, u16 u16ByteNumMax,void* pPrivate)
{
    pDataAddr[0] = (u8)((stModbusSlaveHoldReg.u16HoldReg[8] & 0xFF00) >> 8);
    pDataAddr[1] = (u8)(stModbusSlaveHoldReg.u16HoldReg[8] & 0x00FF);
    pDataAddr[2] = (u8)((stModbusSlaveHoldReg.u16HoldReg[9] & 0xFF00) >> 8);
    pDataAddr[3] = (u8)(stModbusSlaveHoldReg.u16HoldReg[9] & 0x00FF);
    pDataAddr[4] = (u8)((stModbusSlaveHoldReg.u16HoldReg[10] & 0xFF00) >> 8);
    pDataAddr[5] = (u8)(stModbusSlaveHoldReg.u16HoldReg[10] & 0x00FF);
    pDataAddr[6] = (u8)((stModbusSlaveHoldReg.u16HoldReg[11] & 0xFF00) >> 8);
    pDataAddr[7] = (u8)(stModbusSlaveHoldReg.u16HoldReg[11] & 0x00FF);
    return 0;    
}
static s32 HoldRegisters_Table_SetValueCallback_8_11(u16 ParmEnumName, const u8* pDataAddr, u32 u32StartIndex, u16 u16ByteNumMax, void* pPrivate)
{
    stModbusSlaveHoldReg.u16HoldReg[8]  = ((u16)pDataAddr[0]<<8) | (u16)pDataAddr[1];
    stModbusSlaveHoldReg.u16HoldReg[9]  = ((u16)pDataAddr[2]<<8) | (u16)pDataAddr[3];
    stModbusSlaveHoldReg.u16HoldReg[10] = ((u16)pDataAddr[4]<<8) | (u16)pDataAddr[5];
    stModbusSlaveHoldReg.u16HoldReg[11] = ((u16)pDataAddr[6]<<8) | (u16)pDataAddr[7];
    return 0;
}

/* 只读寄存器回调函数 */
static s32 InputRegisters_Table_GetValueCallback_6_12(u16 ParmEnumName, u8* pDataAddr, u32 u32StartIndex, u16 u16ByteNumMax, void* pPrivate)
{
    pDataAddr[0] = (u8)((stModbusSlaveInputReg.u16InputReg[6] & 0xFF00) >> 8);
    pDataAddr[1] = (u8)(stModbusSlaveInputReg.u16InputReg[6] & 0x00FF);
    pDataAddr[2] = (u8)((stModbusSlaveInputReg.u16InputReg[7] & 0xFF00) >> 8);
    pDataAddr[3] = (u8)(stModbusSlaveInputReg.u16InputReg[7] & 0x00FF);
    return 0;
}

/* 只读寄存器回调函数 */
static s32 InputRegisters_Table_GetValueCallback_13_13(u16 ParmEnumName, u8* pDataAddr, u32 u32StartIndex, u16 u16ByteNumMax, void* pPrivate)
{
    pDataAddr[0] = (u8)(stModbusSlaveInputReg.u16InputReg[13] & 0x00FF);
    pDataAddr[1] = (u8)((stModbusSlaveInputReg.u16InputReg[13] & 0xFF00) >> 8);
    return 0;
}


/***********************************************************************************************************************
*                                              STATIC VARIABLE DECLARATIONS
***********************************************************************************************************************/
static ProtocolModbusRtuMsgInfoType stProtocolModbusRtuMsgInfo  = {
    .pProtocolCmdConfigObj = NULL, /* 协议命令配置对象 */
 };


static ProtocolElementConfigType Coils_Table[] = {
    {0, 0, NO_DATA_NAME, NULL, NULL}, // 示例参数配置
};

static ProtocolElementConfigType DiscreteInputs_Table[] = {
    {0, 0, NO_DATA_NAME, NULL, NULL}, // 示例参数配置
};

static ProtocolElementConfigType HoldRegisters_Table[] = {
    {0, 2, NO_DATA_NAME, HoldRegisters_Table_GetValueCallback_0_2, HoldRegisters_Table_SetValueCallback_0_2},
    {3, 3, NO_DATA_NAME, HoldRegisters_Table_GetValueCallback_3_3, HoldRegisters_Table_SetValueCallback_3_3},
    {4, 4, NO_DATA_NAME, HoldRegisters_Table_GetValueCallback_4_4, HoldRegisters_Table_SetValueCallback_4_4},
    {5, 6, NO_DATA_NAME, HoldRegisters_Table_GetValueCallback_5_6, HoldRegisters_Table_SetValueCallback_5_6},
    {7, 7, NO_DATA_NAME, HoldRegisters_Table_GetValueCallback_7_7, HoldRegisters_Table_SetValueCallback_7_7},
    {8, 11, NO_DATA_NAME, HoldRegisters_Table_GetValueCallback_8_11, HoldRegisters_Table_SetValueCallback_8_11},
    {12, 13, PROTOCOL_MODBUS_RTU_SlAVE_HOLDING_REGISTERS_DATA6, NULL, NULL},
    {14, 14, PROTOCOL_MODBUS_RTU_SlAVE_HOLDING_REGISTERS_DATA7, NULL, NULL},
};


static ProtocolElementConfigType InputRegisters_Table[] = {
    {0, 1, PROTOCOL_MODBUS_RTU_SlAVE_INPUT_REGISTERS_DATA0, NULL, NULL}, 
    {2, 3, PROTOCOL_MODBUS_RTU_SlAVE_INPUT_REGISTERS_DATA1, NULL, NULL}, 
    {4, 4, PROTOCOL_MODBUS_RTU_SlAVE_INPUT_REGISTERS_DATA2, NULL, NULL}, 
    {5, 5, PROTOCOL_MODBUS_RTU_SlAVE_INPUT_REGISTERS_DATA3, NULL, NULL}, 
    {6, 12, NO_DATA_NAME, InputRegisters_Table_GetValueCallback_6_12, NULL},
    {13, 13, NO_DATA_NAME, InputRegisters_Table_GetValueCallback_13_13, NULL},
    {14, 15, PROTOCOL_MODBUS_RTU_SlAVE_INPUT_REGISTERS_DATA6, NULL, NULL}, 
};


static ProtocolCmdConfigType Protocol_ModbusRtu_CmdConfigTable[] = {
    {MODBUS_FUNCTION_READ_COILS, Coils_Table, sizeof(Coils_Table)/sizeof(Coils_Table[0]), Coils_Table, sizeof(Coils_Table)/sizeof(Coils_Table[0]), NULL},
    {MODBUS_FUNCTION_READ_DISCRETEINPUTS, Coils_Table, sizeof(Coils_Table)/sizeof(Coils_Table[0]), Coils_Table, sizeof(Coils_Table)/sizeof(Coils_Table[0]), NULL},
    {MODBUS_FUNCTION_READ_HOLDINGREGISTERS, HoldRegisters_Table, sizeof(HoldRegisters_Table)/sizeof(HoldRegisters_Table[0]), HoldRegisters_Table, sizeof(HoldRegisters_Table)/sizeof(HoldRegisters_Table[0]), NULL},
    {MODBUS_FUNCTION_READ_INPUTREGISTERS, InputRegisters_Table, sizeof(InputRegisters_Table)/sizeof(InputRegisters_Table[0]), InputRegisters_Table, sizeof(InputRegisters_Table)/sizeof(InputRegisters_Table[0]), NULL},
    {MODBUS_FUNCTION_WRITE_SINGLECOIL, Coils_Table, sizeof(Coils_Table)/sizeof(Coils_Table[0]), Coils_Table, sizeof(Coils_Table)/sizeof(Coils_Table[0]), NULL},
    {MODBUS_FUNCTION_WRITE_SINGLEREGISTER, HoldRegisters_Table, sizeof(HoldRegisters_Table)/sizeof(HoldRegisters_Table[0]), HoldRegisters_Table, sizeof(HoldRegisters_Table)/sizeof(HoldRegisters_Table[0]), NULL},
    {MODBUS_FUNCTION_WRITE_MULTIPLEREGISTERS, HoldRegisters_Table, sizeof(HoldRegisters_Table)/sizeof(HoldRegisters_Table[0]), HoldRegisters_Table, sizeof(HoldRegisters_Table)/sizeof(HoldRegisters_Table[0]), NULL},
};


/***********************************************************************************************************************
*                                              GLOBAL FUNCTION DEFINITIONS
***********************************************************************************************************************/

/***********************************************************************************************************************
*                                              GLOBAL VARIABLE DECLARATIONS
***********************************************************************************************************************/

ModbusSlaveHoldRegType stModbusSlaveHoldReg = {
    .stAppData.u32Data0 = 0x12345678,
    .stAppData.u16Data1 = 0x9ABC,
    .stAppData.u16Data2 = 0xDEF0,
    .stAppData.u32Data3 = 0x12345678,
    .stAppData.u16Data4 = 0x9ABC,
    .stAppData.u64Data5 = 0x123456789ABCDEF0,
    .stAppData.u32Data6 = 0x12345678,
    .stAppData.u16Data7 = 0x9ABC,
};

ModbusSlaveInputRegType stModbusSlaveInputReg = {
    .stAppData.u32Data0 = 0x12345678,
    .stAppData.u32Data1 = 0x9ABCDEF0,
    .stAppData.u16Data2 = 0x1234,
    .stAppData.u16Data3 = 0x5678,
    .stAppData.u32Data4_0 = 0x12345678,
    .stAppData.u32Data4_1 = 0x9ABCDEF0,
    .stAppData.u32Data4_2 = 0x12345678,
    .stAppData.u16Data4_3 = 0x5678,
    .stAppData.u16Data5 = 0x9ABC,
    .stAppData.u32Data6 = 0x12345678,
};

HalfDuplexSlaveProtocolInstanceType  stModbusRtu_Slave_Instance = {
    .u8ProtocolName = MODBUS_RTU,
    .ProtocolCmdConfigTable = Protocol_ModbusRtu_CmdConfigTable,
    .u8ProtocolCmdNum = sizeof(Protocol_ModbusRtu_CmdConfigTable)/sizeof(Protocol_ModbusRtu_CmdConfigTable[0]),
    .u8SlaveAddr = 0x01,
    // .u16PresetTimeOut = 5,
    .pPrivate = (void*)&stProtocolModbusRtuMsgInfo,
    .pProtocolIdentify = Protocol_ModbusRtu_SlaveIdentify,
    .pEncode = Protocol_ModbusRtu_SlaveEncode,
    .pDecode = Protocol_ModbusRtu_SlaveDecode,
    .pErrorProcess = ErrorProcess,
};

