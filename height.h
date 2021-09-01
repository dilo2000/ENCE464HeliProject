/*
 * File: height.h
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
 * Description: Header file for the header module. Contains prototypes to
 * initialise the height task and obtain the height of the helicopter.
 *
 *
 */

#ifndef HEIGHT_H_
#define HEIGHT_H_

//*****************************************************************************
//
// Prototypes for the height module.
//
//*****************************************************************************
uint32_t InitReadHeight(void);
uint32_t GetHeight(void);

#endif /* HEIGHT_H_ */
