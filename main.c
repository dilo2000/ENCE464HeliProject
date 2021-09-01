/*
 * File: main.c
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
 * Description: This script
 *
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

#include "OrbitOLED/OrbitOLEDInterface.h"

#include "button_task.h"
#include "yaw.h"
#include "rotor.h"
#include "height.h"
#include "controller.h"
#include "display.h"
#include "debugger.h"

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}

#endif

//*****************************************************************************
//
// This hook is called by FreeRTOS when an stack overflow error is detected.
//
//*****************************************************************************
void
vApplicationStackOverflowHook(xTaskHandle *pxTask, char *pcTaskName)
{
    //
    // This function can not return, so loop forever.  Interrupts are disabled
    // on entry to this function, so no processor interrupts will interrupt
    // this loop.
    //
    while(1)
    {
    }
}

//*****************************************************************************
//
// Initialize FreeRTOS and start the initial set of tasks.
//
//*****************************************************************************
int
main(void)
{
    //
    // Set the clocking to run at 50 MHz from the PLL.
    //
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                       SYSCTL_OSC_MAIN);



    //Enable interrupts to the processor.
    IntMasterEnable();

    //
    // Create display task
    //
    if (InitDisplayTask() != 0)
    {
        while(1)
        {
        }
    }

    //
    //Create a task to drive the tail rotor.
    //
    if (InitRotorTask() != 0)
    {
        while(1)
        {
        }
    }

    // Create buttons task
    //
    //
    if (InitButtonTask() != 0)
    {
        while(1)
        {
        }
    }

    //
    // Create controller task
    //
    if(InitControllerTask() != 0)
    {
        while(1)
        {
        }
    }

    //
    // Create height task
    //
    if(InitReadHeight() != 0)
    {
        while(1)
        {
        }
    }

    //
    // Create angle task
    //
    if(InitReadAngle() != 0)
    {
        while(1)
        {
        }
    }

    //
    // Create debug task
    //
    if (InitDebugTask() != 0)
    {
        while(1)
        {

        }
    }

    //
    // Start the scheduler.  This should not return.
    //
    vTaskStartScheduler();

    //
    // In case the scheduler returns for some reason, print an error and loop
    // forever.
    //
    while(1)
    {
    }
}
