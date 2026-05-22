/**
 * @file ParameterInfo.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2026-02-05
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef __PROTOCOL_INFO_H__
#define __PROTOCOL_INFO_H__

#include "ParameterConfig.h"

/***********************************************************************************************************************
*                                            SOURCE FILE VERSION INFORMATION
***********************************************************************************************************************/

/***********************************************************************************************************************
*                                                  FILE VERSION CHECKS
***********************************************************************************************************************/

/***********************************************************************************************************************
*                                                   DEFINES AND MACROS
***********************************************************************************************************************/
#define NO_DATA_NAME 0xffff

/***********************************************************************************************************************
*                                                        ENUMS
***********************************************************************************************************************/
/* 参数名称枚举,自动生成 */
typedef enum{
    /* MODBUS MASTER 数据 */
    // 保持寄存器数据
    // PROTOCOL_MODBUS_RTU_MASTER_HOLDING_REG_DATA0,
    // PROTOCOL_MODBUS_RTU_MASTER_HOLDING_REG_DATA1,
    // PROTOCOL_MODBUS_RTU_MASTER_HOLDING_REG_DATA2,
    // PROTOCOL_MODBUS_RTU_MASTER_HOLDING_REG_DATA3,
    // PROTOCOL_MODBUS_RTU_MASTER_HOLDING_REG_DATA4,
    
    // // 输入寄存器数据
    // PROTOCOL_MODBUS_RTU_MASTER_INPUT_REG_DATA0,
    // PROTOCOL_MODBUS_RTU_MASTER_INPUT_REG_DATA1,
    // PROTOCOL_MODBUS_RTU_MASTER_INPUT_REG_DATA2,
    // PROTOCOL_MODBUS_RTU_MASTER_INPUT_REG_DATA3,
    // PROTOCOL_MODBUS_RTU_MASTER_INPUT_REG_DATA4,
    
    /* MODBUS SLAVES 数据 */
    // 保持寄存器数据
    PROTOCOL_MODBUS_RTU_SlAVE_HOLDING_REGISTERS_DATA6,
    PROTOCOL_MODBUS_RTU_SlAVE_HOLDING_REGISTERS_DATA7,

    // 输入寄存器数据
    PROTOCOL_MODBUS_RTU_SlAVE_INPUT_REGISTERS_DATA0,
    PROTOCOL_MODBUS_RTU_SlAVE_INPUT_REGISTERS_DATA1,
    PROTOCOL_MODBUS_RTU_SlAVE_INPUT_REGISTERS_DATA2,
    PROTOCOL_MODBUS_RTU_SlAVE_INPUT_REGISTERS_DATA3,
    PROTOCOL_MODBUS_RTU_SlAVE_INPUT_REGISTERS_DATA6,
    
    ALL_PARAMETER_NUM,
}ParameterNameType;

/***********************************************************************************************************************
*                                              GLOBAL FUNCTION PROTOTYPES
***********************************************************************************************************************/
extern ParameterConfigType stParameterConfig[];

#endif
