/*
 * File: yaw.h
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
 * Description: Header file for the yaw module. Contains prototypes
 * to initialise a task to read the angle and a to obtain the current
 * yaw.
 *
 *
 */


#ifndef YAW_TASK_H_
#define YAW_TASK_H_

//*****************************************************************************
//
// Prototypes for the yaw module.
//
//*****************************************************************************
int16_t GetYawAngle (void);
uint32_t InitReadAngle (void);

#endif /* YAW_TASK_H_ */

