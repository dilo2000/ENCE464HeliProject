/*
 * File: pid.c
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
 * Description: This module is a general purpose PI module. It uses a struct of type PI which has five variables:
 * kp: Proportional gain
 * ki: Integral gain
 * accum: accumulator for integral
 * limit: limit for integral to prevent wind-up
 * error: current error input
 *
 * NOTE: This module was adapted from "464 SOLID Principles" by Dr Ben Mitchell.
 *
 */

#include <stdbool.h>
#include <stdint.h>

#include "pid.h"


//*****************************************************************************
//
// Initialise a PI instance with appropriate gains.
//
//*****************************************************************************
void pi_init(PI *pi, uint8_t kp, uint8_t ki, uint8_t limit)
{
    pi->kp = kp;
    pi->ki = ki;
    pi->limit = limit;
    pi->accum = 0;
    pi->error = 0;
}

//*****************************************************************************
//
// Update a PI controller with a new error.
//
//*****************************************************************************
void
pi_update(PI *pi, int16_t error, uint16_t dt)
{

    pi->accum += error * dt;

    if (pi->accum > pi->limit)
    {
        pi->accum = pi->limit;
    }
    if (pi->accum < -pi->limit)
    {
        pi->accum = -pi->limit;
    }

    pi->error = error;
}

//*****************************************************************************
//
// Retrieve the current output command from the PI controller.
//
//*****************************************************************************
int16_t
pi_get(PI *pi)
{
    return pi->kp * pi->error + pi->ki * pi->accum;
}
