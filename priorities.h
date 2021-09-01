/*
 * File: priorities.h
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
 * Description: This header file specifies the priorities of each
 * FreeRTOS task.
 *
 *
 */

#ifndef PRIORITIES_H_
#define PRIORITIES_H_

//*****************************************************************************
//
// The priorities of the various tasks.
//
//*****************************************************************************
#define BUTTONTASKPRIORITY         1
#define CONTROLLERTASKPRIORITY     1
#define DEBUGTASKPRIORITY          1
#define DISPLAYTASKPRIORITY        1
#define ADCTASKPRIORITY            2
#define PERIODICTASKPRIORITY       1
#define ROTORTASKPRIORITY          1
#define YAWTASKPRIORITY            3

#endif /* PRIORITIES_H_ */
