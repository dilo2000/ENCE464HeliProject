/*
 * File: controller.h
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
 * Description: Header file for the controller module. Contains functions to initialise the
 * controller task, get the yaw error and update the duty cycle.
 *
 *
 */

#ifndef CONTROLLER_TASK_H_
#define CONTROLLER_TASK_H_

//*****************************************************************************
//
// Prototypes for the controller module.
//
//*****************************************************************************
void vControlInit(void);
void vControlUpdate(int16_t);
uint16_t ui16ControlGet();
int16_t i16GetError(uint16_t, int16_t);
uint32_t InitControllerTask(void);

#endif /* CONTROLLER_TASK_H_ */

