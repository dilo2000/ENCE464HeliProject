/*
 * File: rotor.h
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
 * Description: Header file for the rotor module. Contains
 * prototypes to initialise the rotor task and set the duty
 * and frequency of the rotors.
 *
 *
 */


#ifndef TAIL_DRIVE_TASK_H_
#define TAIL_DRIVE_TASK_H_

//*****************************************************************************
//
// Struct to hold the rotor duty.
//
//*****************************************************************************
typedef struct {
       uint16_t       tailDuty;
       uint16_t       mainDuty;
   } MOTOR_OUTPUT;

//*****************************************************************************
//
// Prototypes for the Tail Rotor task.
//
//*****************************************************************************
uint32_t InitRotorTask (void);
void vSetMotorOutputs (uint16_t, uint16_t);

//*****************************************************************************
//
// Queue for the tail rotor duty.
//
//*****************************************************************************
xQueueHandle g_pRotorQueue;

#endif /* TAIL_DRIVE_TASK_H_ */

