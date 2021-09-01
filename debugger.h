/*
 * File: debugger.h
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
 * Description: This module is responsible for accessing the UART peripheral. It uses a queue
 * to receive data to be printed from other modules.
 *
 * Adapted from freertos_demo.c - Simple FreeRTOS example.
 *
 */

#ifndef DEBUG_H_
#define DEBUG_H_


//*****************************************************************************
//
// Struct for sending data to print to the UART.
//
//*****************************************************************************
typedef enum {
    YAW,
    YAWREF,
    HEIGHT,
    HEIGHTREF,
    STATE,
    DUTY
} DebugSource;

typedef struct {
       uint32_t       Value;
       DebugSource    Source;
   } DEBUG_VALUES;


//*****************************************************************************
//
// Creating a mutex to protect the UART peripheral.
//
//*****************************************************************************
SemaphoreHandle_t xUARTSemaphore;

//*****************************************************************************
//
// Prototypes for debug module.
//
//*****************************************************************************
uint32_t InitDebugTask (void);
void SendToDebugger (uint16_t, DebugSource);


//*****************************************************************************
//
// Queue and Semaphore to print to the UART.
//
//*****************************************************************************
xQueueHandle g_pDebugQueue;

#endif /* DEBUG_H_ */


