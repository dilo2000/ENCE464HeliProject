/*
 * File: fsm.c
 * Project: ENCE464 Assignment 1
 *
 * Authors:
 * - Oliver Dale
 * - Josh Roberts
 * - Micaela Cooper
 * - Angus Fairbairn
 *
 * Created on: 28.08.21
 *
 * Description: This module operates the finite state machine for the helicopter control system.
 * There are four states: IDLE, TAKEOFF, FLYING, LANDING
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "pid.h"
#include "controller.h"
#include "yaw.h"
#include "button_task.h"
#include "rotor.h"
#include "height.h"
#include "debugger.h"
#include "fsm.h"

typedef State (*state_fn)(void);

uint32_t currentHeight = 0;
uint16_t tailDuty;
uint16_t mainDuty;

State StateIdle(void)
{
    tailDuty = 0;
    mainDuty = 0;
    vSetMotorOutputs(tailDuty, mainDuty);
    if (GetButPushed() == 1)
    {
        return(TAKEOFF);
    }
    return(IDLE);
}

State StateTakeoff(void)
{
    tailDuty = 74;
    mainDuty = 90;
    vSetMotorOutputs(tailDuty, mainDuty);
    if (GetHeight() > 50)
    {
        return (FLYING);
    }
    return(TAKEOFF);
}

State StateFlying(void)
{
    int16_t i16Error = 0;

    i16Error = i16GetError(GetRefYaw(), GetYawAngle());

    tailDuty = ui16ControlUpdate(i16Error);
    mainDuty = 90;
    vSetMotorOutputs(tailDuty, mainDuty);

    if (GetRefHeight() == 0)
    {
        return(LANDING);
    }

    return(FLYING);
}

State StateLanding(void)
{
    tailDuty = 50;
    mainDuty = 30;
    vSetMotorOutputs(tailDuty, mainDuty);
    if (GetHeight() == 0)
    {
        return(IDLE);
    }
    return(LANDING);
}

state_fn state_table[] = {
  &StateIdle,
  &StateTakeoff,
  &StateFlying,
  &StateLanding
};

State state = IDLE;
void fsm_update()
{
    state = state_table[state]();
    SendToDebugger (state, STATE);
}
