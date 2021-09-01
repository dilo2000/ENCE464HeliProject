/*
 * File: debugger.c
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

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/pwm.h"
#include "driverlib/interrupt.h"
#include "drivers/buttons.h"
#include "driverlib/adc.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "debugger.h"
#include "priorities.h"
#include "fsm.h"

//*****************************************************************************
//
// The stack size for the debug task.
//
//*****************************************************************************
#define DEBUGTASKSTACKSIZE          128

//*****************************************************************************
//
// Defining debug queue length and item size.
//
//*****************************************************************************
#define DEBUG_ITEM_SIZE           sizeof(DEBUG_VALUES)
#define DEBUG_QUEUE_SIZE          10

//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void
ConfigureUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}

//*****************************************************************************
//
// Receives debug info from other modules and prints to the UART.
//
//*****************************************************************************
static void
DebugTask (void *pvParameters)
{
    uint32_t ui32DebugDelay = 25;   //task operates every 1000 ms

    portTickType ui16DelayTime;
    ui16DelayTime = xTaskGetTickCount();

    DEBUG_VALUES   SERIAL_VALUE;

    static uint16_t ui16Yaw = 0;
    static uint16_t ui16RefYaw = 0;
    static uint16_t ui16Height = 0;
    static uint16_t ui16RefHeight = 0;
    static uint16_t ui16State = 0;
    static uint16_t ui16Duty = 0;

    while (1)
    {
        if(xQueueReceive(g_pDebugQueue, &(SERIAL_VALUE), 0) == pdPASS)
        {
            switch (SERIAL_VALUE.Source) {
                case YAW:
                    ui16Yaw = SERIAL_VALUE.Value;
                    break;
                case YAWREF:
                    ui16RefYaw = SERIAL_VALUE.Value;
                    break;
                case HEIGHT:
                    ui16Height = SERIAL_VALUE.Value;
                    break;
                case HEIGHTREF:
                    ui16RefHeight = SERIAL_VALUE.Value;
                    break;
                case STATE:
                    ui16State = SERIAL_VALUE.Value;
                    break;
                case DUTY:
                    ui16Duty = SERIAL_VALUE.Value;
                    break;
            }


            //
            // Guard UART from concurrent access.
            //
            xSemaphoreTake(xUARTSemaphore, portMAX_DELAY);
            UARTprintf("Current Yaw: %d\n", ui16Yaw);
            UARTprintf("Reference Yaw: %d\n", ui16RefYaw);
            UARTprintf("Current Height: %d\n", ui16Height);
            UARTprintf("Reference Height: %d\n", ui16RefHeight);

            if (ui16State == IDLE)
            {
                UARTprintf("Current State: IDLE\n");
            }
            else if (ui16State == TAKEOFF)
            {
                UARTprintf("Current State: TAKEOFF\n");
            }
            else if (ui16State == FLYING)
            {
                UARTprintf("Current State: FLYING\n");
            }
            else if (ui16State == LANDING)
            {
                UARTprintf("Current State: LANDING\n");
            }
            UARTprintf("Tail Duty: %d\n", ui16Duty);
            UARTprintf("\n");

            xSemaphoreGive(xUARTSemaphore);
        }

//        vTaskDelayUntil(&ui16DelayTime, ui32DebugDelay / portTICK_RATE_MS);
    }
}

void
SendToDebugger (uint16_t Value, DebugSource Source)
{
    DEBUG_VALUES DebugStruct;

    DebugStruct.Source = Source;
    DebugStruct.Value = Value;

    // Pass the value of the reference angle to the controller.
    if(xQueueSend(g_pDebugQueue, &DebugStruct, portMAX_DELAY) !=
            pdPASS)
    {
        UARTprintf("\nQueue full. This should never happen.\n");
        while(1)
        {
            //
            // Error. The queue should never be full. If so print the
            // error message on UART and wait for ever.
            //
        }
    }
}

//*****************************************************************************
//
// Initializes the Debugger task.
//
//*****************************************************************************
uint32_t
InitDebugTask (void)
{
    //
    // Initialize the UART and configure it for 115,200, 8-N-1 operation.
    //
    ConfigureUART();

    //
    // Create a mutex to guard the UART.
    //
    xUARTSemaphore = xSemaphoreCreateMutex();
    if( xUARTSemaphore != NULL )
    {
        // The semaphore was created successfully.
        // The semaphore can now be used.
    }

    //
    // Create a series of queues for sending messages to the display task.
    //
    g_pDebugQueue = xQueueCreate(DEBUG_QUEUE_SIZE, DEBUG_ITEM_SIZE);

    if(xTaskCreate(DebugTask, (const portCHAR *)"Debug",
                       DEBUGTASKSTACKSIZE, NULL,  tskIDLE_PRIORITY + DEBUGTASKPRIORITY, NULL) != pdTRUE)
    {
        return(1);
    }

    return(0);
}

