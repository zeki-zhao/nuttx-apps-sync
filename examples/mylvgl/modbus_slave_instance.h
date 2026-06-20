/**
 * @file Protocol_ModbusRtu_Slave_Instance.h
 * @author zk (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2026-03-19
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef __PROTOCOL_MODBUSRTU_SLAVE_INSTANCE_H__
#define __PROTOCOL_MODBUSRTU_SLAVE_INSTANCE_H__

#include "ProtocolManager.h"


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

typedef union{
    u16 u16HoldReg[20];
    struct {
        u32 u32Data0;
        u16 u16Data1;
        u16 u16Data2;
        u32 u32Data3;
        u16 u16Data4;
        u64 u64Data5;
        u32 u32Data6;
        u16 u16Data7;
    }stAppData;
}ModbusSlaveHoldRegType;

typedef union{
    u16 u16InputReg[20];
    struct {
        u32 u32Data0;
        u32 u32Data1;
        u16 u16Data2;
        u16 u16Data3;
        u32 u32Data4_0;
        u32 u32Data4_1;
        u32 u32Data4_2;
        u16 u16Data4_3;
        u16 u16Data5;
        u32 u32Data6;
    }stAppData;
}ModbusSlaveInputRegType;


extern ModbusSlaveHoldRegType stModbusSlaveHoldReg;
extern ModbusSlaveInputRegType stModbusSlaveInputReg;
extern HalfDuplexSlaveProtocolInstanceType  stModbusRtu_Slave_Instance;

/***********************************************************************************************************************
*                                              GLOBAL FUNCTION PROTOTYPES
***********************************************************************************************************************/


#endif //__PROTOCOL_MODBUSRTU_SLAVE_INSTANCE_H__