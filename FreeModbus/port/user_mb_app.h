#ifndef    USER_APP
#define USER_APP
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
//#include "mb_m.h"
#include "mbconfig.h"
#include "mbframe.h"
#include "mbutils.h"

/* -----------------------Slave Defines -------------------------------------*/
#define S_DISCRETE_INPUT_START        0
#define S_DISCRETE_INPUT_NDISCRETES   16      //输入开关量
#define S_COIL_START                  0
#define S_COIL_NCOILS                 24      //输出开关量
#define S_REG_INPUT_START             0
#define S_REG_INPUT_NREGS             10     //输入模拟量
#define S_REG_HOLDING_START           0 
#define S_REG_HOLDING_NREGS           50     //保持寄存器
/* salve mode: holding register's all address */
#define          S_HD_RESERVE                     0
#define          S_HD_CPU_USAGE_MAJOR             1
#define          S_HD_CPU_USAGE_MINOR             2
/* salve mode: input register's all address */
#define          S_IN_RESERVE                     0
/* salve mode: coil's all address */
#define          S_CO_RESERVE                     0
/* salve mode: discrete's all address */
#define          S_DI_RESERVE                     0

/* -----------------------Master Defines -------------------------------------*/
// #define M_DISCRETE_INPUT_START        0
// #define M_DISCRETE_INPUT_NDISCRETES   16
// #define M_COIL_START                  0
// #define M_COIL_NCOILS                 24
// #define M_REG_INPUT_START             0
// #define M_REG_INPUT_NREGS             20
// #define M_REG_HOLDING_START           0
// #define M_REG_HOLDING_NREGS           20
// /* master mode: holding register's all address */
// #define          M_HD_RESERVE                     0
// /* master mode: input register's all address */
// #define          M_IN_RESERVE                     0
// /* master mode: coil's all address */
// #define          M_CO_RESERVE                     0
// /* master mode: discrete's all address */
// #define          M_DI_RESERVE                     0



#endif
