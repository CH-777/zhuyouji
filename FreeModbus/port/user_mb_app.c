#include "user_mb_app.h"
#include <inttypes.h>

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "output_gpio.h"
#include "sys.h"

#include "oil.h"
#include "elog.h"

/*------------------------Slave mode use these variables----------------------*/
//Slave mode:DiscreteInputs variables
USHORT usSDiscInStart = S_DISCRETE_INPUT_START;
    
#if S_DISCRETE_INPUT_NDISCRETES%8
UCHAR ucSDiscInBuf[S_DISCRETE_INPUT_NDISCRETES/8+1];
#else
UCHAR ucSDiscInBuf[S_DISCRETE_INPUT_NDISCRETES/8];
#endif


//Slave mode:Coils variables
USHORT usSCoilStart = S_COIL_START;
#if S_COIL_NCOILS%8
UCHAR    ucSCoilBuf[S_COIL_NCOILS/8+1];
#else
UCHAR ucSCoilBuf[S_COIL_NCOILS/8] = {0xFF, 0xFF, 0xFF};    //默认全部关闭 ,ON(0) OFF(1)
#endif


//Slave mode:InputRegister variables
USHORT usSRegInStart = S_REG_INPUT_START;
USHORT usSRegInBuf[S_REG_INPUT_NREGS];


//Slave mode:HoldingRegister variables
USHORT usSRegHoldStart = S_REG_HOLDING_START;
USHORT usSRegHoldBuf[S_REG_HOLDING_NREGS];
/*************************************************************************/
//向输入寄存器中写数据,一路占用两个寄存器
void WriteSRegInBuf(uint8_t index, uint32_t value)
{
    taskENTER_CRITICAL();
	usSRegInBuf[index] = value>>16;
	usSRegInBuf[index + 1] = value & 0xFFFF;
	taskEXIT_CRITICAL();
}

/**************************************************************************/
//保持寄存器数组的定义见ref_def.h文件
static void write_oil_add_start_holdreg_cb(void)  /*0*/
{
	if(usSRegHoldBuf[0] != 0)
		add_oil();
}

static void write_formula_index_holdreg_cb(void)  /*6*/
{
    formula_event_post(EV_FORMULA_INDEX_GET);
    log_i("EV_FORMULA_INDEX_GET post");
}
static void write_formula_holdreg_cb(void)     /*8*/
{
    formula_event_post(EV_FORMULA_SAVE);
    log_i("EV_FORMULA_SAVE post");
}
static void write_select_formula_holdreg_cb(void)  /*7*/
{
    formula_event_post(EV_FORMULA_SELECT);
    log_i("EV_FORMULA_SELECT post");
}

typedef void (*CallBackFunc)(void);
//保持寄存器写入回调函数,当向保持寄存器写入新值，调用对应的回调函数，发送相应的事件
CallBackFunc WriteSRegHoldCallBack[S_REG_HOLDING_NREGS] = {
	write_oil_add_start_holdreg_cb,  /*0        启动注油  */ 
	NULL/*1*/, NULL/*2*/, NULL/*3*/, NULL/*4*/, NULL/*5*/, 
	write_formula_index_holdreg_cb/*6*/, 
    write_select_formula_holdreg_cb/*7*/,
    write_formula_holdreg_cb,        /*8        油品       */          
    write_formula_holdreg_cb, NULL,  /*9 -10    压力       */
	write_formula_holdreg_cb, NULL,  /*11-12    温度       */
	write_formula_holdreg_cb, NULL,  /*13-14    密度       */
	write_formula_holdreg_cb, NULL,  /*15-16    设定注油量 */
	write_formula_holdreg_cb, NULL,  /*17-18    设定补偿量 */
	NULL,NULL,NULL,NULL,
};



/**
 * Modbus slave input register callback function.
 *
 * @param pucRegBuffer input register buffer
 * @param usAddress input register address
 * @param usNRegs input register number
 *
 * @return result
 */
eMBErrorCode eMBRegInputCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    USHORT          iRegIndex;
    USHORT *        pusRegInputBuf;
    USHORT          REG_INPUT_START;
    USHORT          REG_INPUT_NREGS;
    USHORT          usRegInStart;

    pusRegInputBuf = usSRegInBuf;//
    REG_INPUT_START = S_REG_INPUT_START;
    REG_INPUT_NREGS = S_REG_INPUT_NREGS;
    usRegInStart = usSRegInStart;

    /* it already plus one in modbus function method. */
    usAddress--;

    if ((usAddress >= REG_INPUT_START)
            && (usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS))
    {
        iRegIndex = usAddress - usRegInStart;
        while (usNRegs > 0)
        {
            *pucRegBuffer++ = (UCHAR) (pusRegInputBuf[iRegIndex] >> 8);
            *pucRegBuffer++ = (UCHAR) (pusRegInputBuf[iRegIndex] & 0xFF);
            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}

/**
 * Modbus slave holding register callback function.
 *
 * @param pucRegBuffer holding register buffer
 * @param usAddress holding register address
 * @param usNRegs holding register number
 * @param eMode read or write
 *
 * @return result
 */
eMBErrorCode eMBRegHoldingCB(UCHAR * pucRegBuffer, USHORT usAddress,
        USHORT usNRegs, eMBRegisterMode eMode)
{
    eMBErrorCode    eStatus = MB_ENOERR;
    USHORT          iRegIndex;
    USHORT *        pusRegHoldingBuf;
    USHORT          REG_HOLDING_START;
    USHORT          REG_HOLDING_NREGS;
    USHORT          usRegHoldStart;

    pusRegHoldingBuf = usSRegHoldBuf;
    REG_HOLDING_START = S_REG_HOLDING_START;
    REG_HOLDING_NREGS = S_REG_HOLDING_NREGS;
    usRegHoldStart = usSRegHoldStart;

    /* it already plus one in modbus function method. */
    usAddress--;

    if ((usAddress >= REG_HOLDING_START)
            && (usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS))
    {
        iRegIndex = usAddress - usRegHoldStart;
        switch (eMode)
        {
        /* read current register values from the protocol stack. */
        case MB_REG_READ:
            while (usNRegs > 0)
            {
                *pucRegBuffer++ = (UCHAR) (pusRegHoldingBuf[iRegIndex] >> 8);
                *pucRegBuffer++ = (UCHAR) (pusRegHoldingBuf[iRegIndex] & 0xFF);
                iRegIndex++;
                usNRegs--;
            }
            break;

        /* write current register values with new values from the protocol stack. */
        case MB_REG_WRITE:
            while (usNRegs > 0)
            {
                pusRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                pusRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                /******************************************************/
                if(WriteSRegHoldCallBack[iRegIndex] != NULL)
				     WriteSRegHoldCallBack[iRegIndex]();   //执行对应的回调函数
                
                /******************************************************/
                iRegIndex++;
                usNRegs--;
            }			
            break;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

/**
 * Modbus slave coils callback function.
 *
 * @param pucRegBuffer coils buffer
 * @param usAddress coils address
 * @param usNCoils coils number
 * @param eMode read or write
 *
 * @return result
 */
eMBErrorCode eMBRegCoilsCB(UCHAR * pucRegBuffer, USHORT usAddress,
        USHORT usNCoils, eMBRegisterMode eMode)
{
    eMBErrorCode    eStatus = MB_ENOERR;
    USHORT          iRegIndex , iRegBitIndex , iNReg;
    UCHAR *         pucCoilBuf; 
    USHORT          COIL_START; 
    USHORT          COIL_NCOILS;
    USHORT          usCoilStart;
    iNReg =  usNCoils / 8 + 1;

    pucCoilBuf = ucSCoilBuf;
    COIL_START = S_COIL_START;
    COIL_NCOILS = S_COIL_NCOILS;
    usCoilStart = usSCoilStart;

    /* it already plus one in modbus function method. */
    usAddress--;

    if( ( usAddress >= COIL_START ) &&
        ( usAddress + usNCoils <= COIL_START + COIL_NCOILS ) )
    {
        iRegIndex = (USHORT) (usAddress - usCoilStart) / 8;
        iRegBitIndex = (USHORT) (usAddress - usCoilStart) % 8;
        switch ( eMode )
        {
        /* read current coil values from the protocol stack. */
        case MB_REG_READ:
			/***************************************************************************/
			get_output_gpios_level(usAddress, usNCoils, pucCoilBuf);
		/********************************************************************************/
            while (iNReg > 0)
            {
                *pucRegBuffer++ = xMBUtilGetBits(&pucCoilBuf[iRegIndex++],
                        iRegBitIndex, 8);
                iNReg--;
            }
            pucRegBuffer--;
            /* last coils */
            usNCoils = usNCoils % 8;
            /* filling zero to high bit */
            *pucRegBuffer = *pucRegBuffer << (8 - usNCoils);
            *pucRegBuffer = *pucRegBuffer >> (8 - usNCoils);
            break;

            /* write current coil values with new values from the protocol stack. */
        case MB_REG_WRITE:
            /*************************************************/		
			if(get_run_mode() == manual_running)    //只有手动运行模式下，写IO才有效 
				set_output_gpios_level(usAddress, usNCoils, pucRegBuffer);/********/
            /*************************************************/
            while (iNReg > 1)
            {
                xMBUtilSetBits(&pucCoilBuf[iRegIndex++], iRegBitIndex, 8,
                        *pucRegBuffer++);
				
                iNReg--;
            }
            /* last coils */
            usNCoils = usNCoils % 8;
            /* xMBUtilSetBits has bug when ucNBits is zero */
            if (usNCoils != 0)
            {
                xMBUtilSetBits(&pucCoilBuf[iRegIndex++], iRegBitIndex, usNCoils,
                        *pucRegBuffer++);
            }
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

/**
 * Modbus slave discrete callback function.
 *
 * @param pucRegBuffer discrete buffer
 * @param usAddress discrete address
 * @param usNDiscrete discrete number
 *
 * @return result
 */
eMBErrorCode eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    USHORT          iRegIndex , iRegBitIndex , iNReg;
    UCHAR *         pucDiscreteInputBuf;
    USHORT          DISCRETE_INPUT_START;
    USHORT          DISCRETE_INPUT_NDISCRETES;
    USHORT          usDiscreteInputStart;
    iNReg =  usNDiscrete / 8 + 1;

    pucDiscreteInputBuf = ucSDiscInBuf;
    DISCRETE_INPUT_START = S_DISCRETE_INPUT_START;
    DISCRETE_INPUT_NDISCRETES = S_DISCRETE_INPUT_NDISCRETES;
    usDiscreteInputStart = usSDiscInStart;

    /* it already plus one in modbus function method. */
    usAddress--;

    if ((usAddress >= DISCRETE_INPUT_START)
            && (usAddress + usNDiscrete    <= DISCRETE_INPUT_START + DISCRETE_INPUT_NDISCRETES))
    {			
        iRegIndex = (USHORT) (usAddress - usDiscreteInputStart) / 8;
        iRegBitIndex = (USHORT) (usAddress - usDiscreteInputStart) % 8;

        while (iNReg > 0)
        {
            *pucRegBuffer++ = xMBUtilGetBits(&pucDiscreteInputBuf[iRegIndex++],
                    iRegBitIndex, 8);
            iNReg--;
        }
        pucRegBuffer--;
        /* last discrete */
        usNDiscrete = usNDiscrete % 8;
        /* filling zero to high bit */
        *pucRegBuffer = *pucRegBuffer << (8 - usNDiscrete);
        *pucRegBuffer = *pucRegBuffer >> (8 - usNDiscrete);
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}






