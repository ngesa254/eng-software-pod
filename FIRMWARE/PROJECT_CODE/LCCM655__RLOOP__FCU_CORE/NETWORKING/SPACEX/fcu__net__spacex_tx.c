/**
 * @file		FCU__NET__SPACEX_TX.C
 * @brief		SpaceX specific network transmission
 * @author		Lachlan Grogan
 * @copyright	rLoop Inc.
 */
/**
 * @addtogroup RLOOP
 * @{ */
/**
 * @addtogroup FCU
 * @ingroup RLOOP
 * @{ */
/**
 * @addtogroup FCU__CORE_NET_SPACEX
 * @ingroup FCU
 * @{ */

#include "../../fcu_core.h"
#if C_LOCALDEF__LCCM655__ENABLE_ETHERNET == 1U
#if C_LOCALDEF__LCCM655__ENABLE_SPACEX_TELEMETRY == 1U

extern struct _strFCU sFCU;

//locals
Luint8 u8FCU_NET_SPACEX_TX__GeneratePodStatus(void);

/***************************************************************************//**
 * @brief
 * Init the spaceX timed network transmission process
 * 
 * @st_funcMD5		E26089EA43BEAD5D8FCFEC4B17029C33
 * @st_funcID		LCCM655R0.FILE.039.FUNC.001
 */
void vFCU_NET_SPACEX_TX__Init(void)
{
	//Clear the timer flag
	sFCU.sSpaceX.u8100MS_Flag = 0U;
	sFCU.sSpaceX.u32TestCounter = 0U;

}

/* 1.3. Determine pod status for SpaceX telemetry
 * As required in section "Pod Monitoring Telemetry" of the Network Guide, the FCU shall compute the pod status as
 * follows and report it to SpaceX telemetry:
 *
 * 0 (Fault): if mission phase is test phase and tests failed is true (see 11. Execute Internal and Functional Tests),
 * or if mission phase is pre-run phase or pusher interlock phase and a fault is detected
 * (As per network guide, the goal of this status is to abort the tube run if needed, therefore it not relevant to report it after Pusher Interlock phase)
 *
 * 2 (Ready): if mission phase is pre-run phase and ready for push state to true
 *
 * 3 (Pushing): if mission phase is pusher interlock phase and pushing state to true
 * (Pushing state becoming false does not mean that brake interlock is released. To be released, the FCU must exit Pusher Interlock mission phase as described above.)
 *
 * 4 (Coast): if (mission phase is pusher interlock phase or flight phase) and coast state to true
 *
 * 5: (Braking): if (mission phase is flight phase or post-run phase) and braking state to true
 *
 * 1: (Idle): otherwise
 *
 */
/***************************************************************************//**
 * @brief
 * ToDo
 * 
 * @st_funcMD5		20488A965D31CF39F54286BC6B95BD52
 * @st_funcID		LCCM655R0.FILE.039.FUNC.004
 */
Luint8 u8FCU_NET_SPACEX_TX__GeneratePodStatus(void)
{
	Luint8 u8Return;

	u8Return = 1;

	return u8Return;
}

/***************************************************************************/
/**
 * @brief
 * Do the transmission
 *
 * @note
 * Teams shall send a binary telemetry frame for SpaceX monitoring of the status of their pod. The frame
 * shall be sent via UDP to IP 192.168.0.1, port 3000 at no greater than 10Hz, and must obey the following
 * format, where the type UINTx is an unsigned integer of x bits in length, and INTx is a twos-complement
 * signed integer of x bits in length. The first 5 fields are required; the latter five fields are optional. If a
 * field is unused, its value must be left as 0.
 *
 * Name             Type            Description
 * team_id          UINT8           Identifier for the team, assigned by SpaceX. Required.
 * status           UINT8           Pod status, indicating current pod health and pushing state, as defined below. Required.
 * acceleration     INT32           Acceleration in centimeters per second squared. Required.
 * position         INT32           Velocity in centimeters per second. Required.
 * velocity         INT32           Position in centimeters. Required.
 * battery_voltage  INT32           Battery voltage in millivolts.
 * battery_current  INT32           Battery current in milliamps.
 * battery_temperature INT32        Battery temperature in tenths of a degree Celsius.
 * pod_temperature  INT32           Pod temperature in tenths of a degree Celsius.
 * stripe_count     UINT32          Count of optical navigation stripes detected in the tube.
 *
 * @st_funcMD5		0FF9F7AA25696D8694628162EBB1F00B
 * @st_funcID       LCCM655R0.FILE.039.FUNC.002
 */
void vFCU_NET_SPACEX_TX__Process(void)
{
    Luint8 u8Test;
    Luint16 u16PacketLength;
    Lint16 s16Return;
    Luint32 u32Buffer;
    Luint8* pu8Return;
    Lint32 s32Temp;
    Lfloat32 f32Temp;

    //do we have a timer flag?
    if(sFCU.sSpaceX.u8100MS_Flag == 1U)
    {
        //send the UDP stream
        //see if we have space to transmit
        u8Test = u8SIL3_ETH_FIFO__Is_Empty();
        if(u8Test == 1U)
        {
            //add space for the network packet
            u16PacketLength = 34U;
            s16Return = s16SIL3_ETH_FIFO__Push(u16PacketLength);
            if(s16Return >= 0)
            {
                //get a pointer to the buffer
                u32Buffer = u32SIL3_ETH_BUFFERDESC__Get_TxBufferPointer((Luint8) s16Return);

                pu8Return = (Luint8*) u32Buffer;

                //team_id           UINT8           Identifier for the team, assigned by SpaceX. Required.
                pu8Return[0] = 13U;
                pu8Return += 1U;

                //status            UINT8           Pod status, indicating current pod health and pushing state, as defined below. Required.
                pu8Return[0] = u8FCU_NET_SPACEX_TX__GeneratePodStatus();
                pu8Return += 1U;

                //acceleration  INT32           Acceleration in centimeters per second squared. Required.
#if C_LOCALDEF__LCCM655__ENABLE_FLIGHT_CONTROL == 1U
                s32Temp = s32FCU_FCTL_NAV__Get_Accel_mm_ss();
#else
                s32Temp = 0;
#endif
                //convert to cm/ss
                s32Temp /= 10;
                vSIL3_NUM_CONVERT__Array_S32(pu8Return, s32Temp);
                pu8Return += 4U;

                //position      INT32           Velocity in centimeters per second. Required.
#if C_LOCALDEF__LCCM655__ENABLE_FLIGHT_CONTROL == 1U
                s32Temp = s32FCU_FCTL_NAV__Get_Veloc_mm_s();
#else
                s32Temp = 0;
#endif
                //convert to cm/s
                s32Temp /= 10;
                vSIL3_NUM_CONVERT__Array_S32(pu8Return, s32Temp);
                pu8Return += 4U;

                //velocity      INT32           Position in centimeters. Required.
#if C_LOCALDEF__LCCM655__ENABLE_FLIGHT_CONTROL == 1U
                s32Temp = s32FCU_FCTL_NAV__Get_Track_Position_mm();
#else
                s32Temp = 0;
#endif
                //convert to cm
                s32Temp /= 10;
                vSIL3_NUM_CONVERT__Array_S32(pu8Return, s32Temp);
                pu8Return += 4U;

                //battery_voltage   INT32           Battery voltage in millivolts.
                f32Temp = sFCU.sBMS[0].f32PackVoltage;
                f32Temp += sFCU.sBMS[1].f32PackVoltage;

                if(sFCU.sBMS[1].u8Seen == 1U)
                {
                	f32Temp /= 2.0F;
                }
                else
                {
                	//no div
                }

                //convert to mV
                f32Temp *= 1000.0F;

                vSIL3_NUM_CONVERT__Array_S32(pu8Return, (Lint32)f32Temp);
                pu8Return += 4U;

                //battery_current   INT32           Battery current in milliamps.

                f32Temp = sFCU.sBMS[0].f32BatteryCurrent;
                f32Temp += sFCU.sBMS[1].f32BatteryCurrent;
                if(sFCU.sBMS[1].u8Seen == 1U)
                {
                	f32Temp /= 2.0F;
                }
                else
                {
                	//no div
                }

                //convert to mA
                f32Temp *= 1000.0F;

                vSIL3_NUM_CONVERT__Array_S32(pu8Return, (Lint32)f32Temp);
                pu8Return += 4U;

                //battery_temperature INT32         Battery temperature in tenths of a degree Celsius.
                f32Temp = sFCU.sBMS[0].f32AverageTemp;
                f32Temp += sFCU.sBMS[1].f32AverageTemp;
                f32Temp /= 2.0F;

                f32Temp /= 10.0F;

                vSIL3_NUM_CONVERT__Array_S32(pu8Return, (Lint32)f32Temp);
                pu8Return += 4U;

                //pod_temperature   INT32           Pod temperature in tenths of a degree Celsius.
                f32Temp = sFCU.sBMS[0].f32PV_Temp;
				f32Temp += sFCU.sBMS[1].f32PV_Temp;
                if(sFCU.sBMS[1].u8Seen == 1U)
                {
                	f32Temp /= 2.0F;
                }
                else
                {
                	//no div
                }

				f32Temp /= 10.0F;

                vSIL3_NUM_CONVERT__Array_S32(pu8Return, (Lint32)f32Temp);
                pu8Return += 4U;

                //stripe_count  UINT32          Count of optical navigation stripes detected in the tube.
                vSIL3_NUM_CONVERT__Array_U32(pu8Return, sFCU.sSpaceX.u32TestCounter++);
                pu8Return += 4U;

                //send it our SpaceX required port
                vSIL3_ETH_UDP__Transmit(u16PacketLength, 3000U, 3000U);
                //clear the flag only after a send because im sure SpX want the data
                //on the 100ms mark.
                sFCU.sSpaceX.u8100MS_Flag = 0U;
            }//if(s16Return >= 0)
            else
            {
              //FIFO error
            }
        }
        else
        {
            //mo more space
        }

    }

}

/***************************************************************************//**
 * @brief
 * To be called from the 100ms timer routine
 * Sets a flag to allow this module to determine that 100ms has elapsed
 * 
 * @st_funcMD5		D01FDA076F08496417237931291DDA01
 * @st_funcID		LCCM655R0.FILE.039.FUNC.003
 */
void vFCU_NET_SPACEX_TX__100MS_ISR(void)
{
	sFCU.sSpaceX.u8100MS_Flag = 1U;
}

#endif //C_LOCALDEF__LCCM655__ENABLE_SPACEX_TELEMETRY
#ifndef C_LOCALDEF__LCCM655__ENABLE_SPACEX_TELEMETRY
	#error
#endif

#endif //C_LOCALDEF__LCCM653__ENABLE_ETHERNET
/** @} */
/** @} */
/** @} */

