/*
 * File: button_task.h
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
 * Description: Header file for the button_task module. Contains prototypes to
 * initialise the buttons task and obtain reference yaw and height values for
 * the helicopter to fly to.
 *
 *
 */

#ifndef BUTTON_TASK_H_
#define BUTTON_TASK_H_

//*****************************************************************************
//
// Prototypes for the Buttons task.
//
//*****************************************************************************
uint32_t InitButtonTask (void);
int16_t GetRefYaw (void);
int16_t GetRefHeight (void);
uint8_t GetButPushed (void);

#endif /* BUTTON_TASK_H_ */
