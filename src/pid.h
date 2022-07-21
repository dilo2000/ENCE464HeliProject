/*
 * File: pid.h
 * Project: ENCE464 Assignment 1
 *
 * Authors:
 * - Oliver Dale
 * - Josh Roberts
 * - Micaela Cooper
 * - Angus Fairbairn
 *
 *
 *
 * Created on: 28.08.21
 *
 * Description: Header file for the pid module. Definition of PI struct and the three PI controller functions.
 *
 *
 */


#ifndef PID_H_
#define PID_H_


//*****************************************************************************
//
// Struct of type PI which has five variables:
// kp: Proportional gain
// ki: Integral gain
// accum: accumulator for integral
// limit: limit for integral to prevent wind-up
// error: current error input
//
//*****************************************************************************
typedef struct
{
    uint8_t kp, ki;
    uint8_t accum, limit;
    int16_t error;
} PI;

//*****************************************************************************
//
// Initialise a PI instance with appropriate gains.
//
//*****************************************************************************
void pi_init(PI*, uint8_t, uint8_t, uint8_t);

//*****************************************************************************
//
// Update a PI controller with a new error.
//
//*****************************************************************************
void pi_update(PI*, int16_t, uint16_t);

//*****************************************************************************
//
// Retrieve the current output command from the PI controller.
//
//*****************************************************************************
int16_t pi_get(PI*);


#endif /* PID_H_ */

