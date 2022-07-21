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
 * Description: This application controls an emulated helicopter using a PI controller
 * for use on a Tiva launchpad. The height and yaw of the helicopter can be set using
 * the left, right, up and down button presses. The current status of the helicopter
 * is displayed on the OLED BoosterPack. This program has been implemented using the
 * FreeRTOS architecture. Each task is initialised in this script and the scheduler is
 * started.
 *
 * The application contains the following tasks:
 *
 * - Display: Displays the current position and desired position of the helicopter on
 * the OLED display.
 *
 * - Rotor: Drives the PWM to the tail and main rotor of the helicopter.
 *
 * - Buttons: Records the button presses, updating the desired yaw and height of the
 * helicopter in response.
 *
 * - Controller: Controls the state machine of the helicopter and calculates the tail
 * rotor duty cycle using a PI controller.
 *
 * - Height: Reads the height of the helicopter from the ADC using periodically triggered
 * interrupts.
 *
 * - Angle: Reads the yaw of the helicopter. ISR's are triggered at each edge change by
 * the rotary encoder.
 *
 * - Debug: Takes information from the controller, height and angle tasks to print to the
 * UART via a FreeRTOS queue.
 *
 * NOTE: The prototypes vApplicationStackOverflowHook and __error__ have been
 * adapted from freertos_demo.c - Simple FreeRTOS example.
 *
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_ints.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "utils/ustdlib.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

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
