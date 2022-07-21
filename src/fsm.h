/*
 * File: fsm.h
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
 * Description: Header file for the fsm module. Definition of enumerated states.
 *
 *
 */

#ifndef FSM_H_
#define FSM_H_

//*****************************************************************************
//
// Enumeration definition of each state.
//
//*****************************************************************************
typedef enum {
    IDLE,
    TAKEOFF,
    FLYING,
    LANDING
} State;

//*****************************************************************************
//
// Function called frequently by controller.c. Updates current state.
//
//*****************************************************************************
void fsm_update();

#endif /* FSM_H_ */

