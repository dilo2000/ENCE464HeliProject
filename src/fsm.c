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
 * There are four states: IDLE, TAKEOFF, FLYING, LANDING.
 *
 * NOTE: This module was adapted from "464 SOLID Principles" by Dr Ben Mitchell.
 *
 */

#include <stdbool.h>
#include <stdint.h>

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

//*****************************************************************************
//
// Global variables for the state machine.
//
//*****************************************************************************
uint32_t currentHeight = 0;
uint16_t tailDuty;
uint16_t mainDuty;

typedef State (*state_fn)(void);

//*****************************************************************************
//
// IDLE State: both rotors not spinning; transition to TAKEOFF when button pushed
//
//*****************************************************************************
State StateIdle(void)
{
    tailDuty = 0;
    mainDuty = 0;
    vSetMotorOutputs(tailDuty, mainDuty); // Send the duty cycles through a queue to the rotor task.
    if (GetButPushed() == 1)
    {
        return(TAKEOFF);
    }
    return(IDLE);
}

//*****************************************************************************
//
// TAKEOFF State: both rotors set at a fixed PWM so that the helicopter spins
// to find reference yaw and moves upwards; transition to FLYING when
// height is > 50.
//
//*****************************************************************************
State StateTakeoff(void)
{
    // helicopter spins to find reference yaw and increases altitude to 50
    tailDuty = 74;
    mainDuty = 70;
    vSetMotorOutputs(tailDuty, mainDuty); // Send the duty cycles through a queue to the rotor task.
    if (GetHeight() > 50)
    {
        return (FLYING);
    }
    return(TAKEOFF);
}

//*****************************************************************************
//
// FLYING State: PWM for main rotor is fixed, PI controller is used to
// stabilise rotational position of helicopter at the desired angle set by
// controller; transition to LANDING if reference height is set to zero.
//
//*****************************************************************************
State StateFlying(void)
{
    int16_t i16Error = 0;

    i16Error = i16GetError(GetRefYaw(), GetYawAngle());

    vControlUpdate(i16Error); //  Update the duty cycle using the pid controller.
    tailDuty = ui16ControlGet();
    mainDuty = 90;
    vSetMotorOutputs(tailDuty, mainDuty); // Send the duty cycles through a queue to the rotor task.

    if (GetRefHeight() == 0)
    {
        return(LANDING);
    }

    return(FLYING);
}

//*****************************************************************************
//
// LANDING State: Helicopter descends with fixed PWM for tail and main rotors;
// transitions to IDLE when height reaches zero.
//
//*****************************************************************************
State StateLanding(void)
{
    tailDuty = 50;
    mainDuty = 30;
    vSetMotorOutputs(tailDuty, mainDuty); // Send the duty cycles through a queue to the rotor task.
    if (GetHeight() == 0)
    {
        return(IDLE);
    }
    return(LANDING);
}

//*****************************************************************************
//
// State table containing a pointer to each possible state in the same order
// as state enumeration.
//
//*****************************************************************************
state_fn state_table[] = {
  &StateIdle,
  &StateTakeoff,
  &StateFlying,
  &StateLanding
};

//*****************************************************************************
//
// Initialisation of state as IDLE.
//
//*****************************************************************************
State state = IDLE;

//*****************************************************************************
//
// Function called frequently by controller.c. Updates current state.
//
//*****************************************************************************
void fsm_update()
{
    state = state_table[state]();
    SendToDebugger (state, STATE); // Send the current state of the fsm to be logged.
}
