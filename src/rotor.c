/*
 * File: rotor.c
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
 * Description: This module initialises the PWM peripherals for the main
 * and tail rotor. It takes a duty cycle from the controller to set the
 * tail rotor.
 *
 * Note: The prototypes: vSetTailPWM, vSetMainPWM, vInitTailPWM, vInitMainPWM
 * have been adapted from pwmGen.c created by P.J.Bones 20.3.2017.
 *
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/pwm.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "rotor.h"
#include "priorities.h"

//*****************************************************************************
//
// The stack size for the rotor task.
//
//*****************************************************************************
#define ROTORTASKSTACKSIZE        128         // Stack size in words

//*****************************************************************************
//
// The queue length and size for the rotor task.
//
//*****************************************************************************
#define ROTOR_QUEUE_SIZE          10
#define ROTOR_ITEM_SIZE           sizeof(MOTOR_OUTPUT)

//*****************************************************************************
//
// PWM Configuration Details.
//
//*****************************************************************************
#define PWM_START_RATE_HZ  250
#define PWM_RATE_STEP_HZ   50
#define PWM_RATE_MIN_HZ    50
#define PWM_RATE_MAX_HZ    400
#define PWM_FIXED_PC       10
#define PWM_DIVIDER_CODE  SYSCTL_PWMDIV_4
#define PWM_DIVIDER  4

//---Tail Rotor PWM: M1PWM5,PF1
#define PWM_TAIL_BASE        PWM1_BASE
#define PWM_TAIL_GEN         PWM_GEN_2
#define PWM_TAIL_OUTNUM      PWM_OUT_5
#define PWM_TAIL_OUTBIT      PWM_OUT_5_BIT
#define PWM_TAIL_PERIPH_PWM   SYSCTL_PERIPH_PWM1
#define PWM_TAIL_PERIPH_GPIO SYSCTL_PERIPH_GPIOF
#define PWM_TAIL_GPIO_BASE   GPIO_PORTF_BASE
#define PWM_TAIL_GPIO_CONFIG GPIO_PF1_M1PWM5
#define PWM_TAIL_GPIO_PIN    GPIO_PIN_1

//---Main Rotor PWM: M0PWM7,PC5, J4-05
#define PWM_MAIN_BASE        PWM0_BASE
#define PWM_MAIN_GEN         PWM_GEN_3
#define PWM_MAIN_OUTNUM      PWM_OUT_7
#define PWM_MAIN_OUTBIT      PWM_OUT_7_BIT
#define PWM_MAIN_PERIPH_PWM   SYSCTL_PERIPH_PWM0
#define PWM_MAIN_PERIPH_GPIO SYSCTL_PERIPH_GPIOC
#define PWM_MAIN_GPIO_BASE   GPIO_PORTC_BASE
#define PWM_MAIN_GPIO_CONFIG GPIO_PC5_M0PWM7
#define PWM_MAIN_GPIO_PIN    GPIO_PIN_5

//*****************************************************************************
//
// Local prototypes for the Rotor task.
//
//*****************************************************************************
void vSetTailPWM (uint32_t ui32Freq, uint32_t ui32Duty);
void vSetMainPWM (uint32_t ui32Freq, uint32_t ui32Duty);
void vInitTailPWM (void);
void vInitMainPWM (void);
static void RotorTask (void *pvParameters);

//*****************************************************************************
//
// Sets the PWM of the tail rotor with a specified frequency and duty cycle.
//
//*****************************************************************************
void
vSetTailPWM (uint32_t ui32Freq, uint32_t ui32Duty)
{
    // Calculate the PWM period corresponding to the freq.
    uint32_t ui32Period = SysCtlClockGet() / PWM_DIVIDER / ui32Freq;

    PWMGenPeriodSet(PWM_TAIL_BASE, PWM_TAIL_GEN, ui32Period);
    PWMPulseWidthSet(PWM_TAIL_BASE, PWM_TAIL_OUTNUM,
        ui32Period * ui32Duty / 100);
}

//*****************************************************************************
//
// Sets the PWM of the main rotor with a specified frequency and duty cycle.
//
//*****************************************************************************
void
vSetMainPWM (uint32_t ui32Freq, uint32_t ui32Duty)
{
    // Calculate the PWM period corresponding to the freq.
    uint32_t ui32Period = SysCtlClockGet() / PWM_DIVIDER / ui32Freq;

    PWMGenPeriodSet(PWM_MAIN_BASE, PWM_MAIN_GEN, ui32Period);
    PWMPulseWidthSet(PWM_MAIN_BASE, PWM_MAIN_OUTNUM,
        ui32Period * ui32Duty / 100);
}


//*****************************************************************************
//
// Initialising the main rotor PWM.
//
//*****************************************************************************
void
vInitMainPWM (void)
{
    SysCtlPeripheralEnable(PWM_MAIN_PERIPH_PWM);
    SysCtlPeripheralEnable(PWM_MAIN_PERIPH_GPIO);

    GPIOPinConfigure(PWM_MAIN_GPIO_CONFIG);
    GPIOPinTypePWM(PWM_MAIN_GPIO_BASE, PWM_MAIN_GPIO_PIN);

    PWMGenConfigure(PWM_MAIN_BASE, PWM_MAIN_GEN,
                    PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

    // Set the initial PWM parameters
    vSetMainPWM (PWM_START_RATE_HZ, PWM_FIXED_PC);

    PWMGenEnable(PWM_MAIN_BASE, PWM_MAIN_GEN);

    // Disable the output.  Repeat this call with 'true' to turn O/P on.
    PWMOutputState(PWM_MAIN_BASE, PWM_MAIN_OUTBIT, false);
}

//*****************************************************************************
//
// Initialising the tail rotor PWM.
//
//*****************************************************************************
void
vInitTailPWM (void)
{
    //
    // Initialising the tail rotor PWM.
    //
    SysCtlPeripheralEnable(PWM_TAIL_PERIPH_PWM);
    SysCtlPeripheralEnable(PWM_TAIL_PERIPH_GPIO);

    GPIOPinConfigure(PWM_TAIL_GPIO_CONFIG);
    GPIOPinTypePWM(PWM_TAIL_GPIO_BASE, PWM_TAIL_GPIO_PIN);

    PWMGenConfigure(PWM_TAIL_BASE, PWM_TAIL_GEN,
                    PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

    // Set the initial PWM parameters
    vSetTailPWM (PWM_START_RATE_HZ, PWM_FIXED_PC);

    PWMGenEnable(PWM_TAIL_BASE, PWM_TAIL_GEN);

    // Disable the output.  Repeat this call with 'true' to turn O/P on.
    PWMOutputState(PWM_TAIL_BASE, PWM_TAIL_OUTBIT, false);
}

//*****************************************************************************
//
// Gets the updated duty cycle for the tail rotor from the controller.
//
//*****************************************************************************
static void
RotorTask (void *pvParameters)
{
    MOTOR_OUTPUT DutyStruct;
    DutyStruct.mainDuty = 90;

    while (1)
    {
        if(xQueueReceive(g_pRotorQueue, &(DutyStruct), 0) == pdPASS) // Receive the tail rotor duty cycle and check it was successful.
        {
            vSetTailPWM (PWM_START_RATE_HZ, DutyStruct.tailDuty);
            vSetMainPWM (PWM_START_RATE_HZ, DutyStruct.mainDuty);
        }
    }
}

//*****************************************************************************
//
// Sends the PWM of the rotor from a received duty cycle.
//
//*****************************************************************************
void
vSetMotorOutputs(uint16_t tailDuty, uint16_t mainDuty)
{
    MOTOR_OUTPUT DutyStruct;
    DutyStruct.tailDuty = tailDuty;
    DutyStruct.mainDuty = mainDuty;

    // Pass the value of the duty cycle to the rotor task.
    if(xQueueSend(g_pRotorQueue, &(DutyStruct), portMAX_DELAY) !=
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
// Initializes the Rotor task.
//
//*****************************************************************************
uint32_t
InitRotorTask (void)
{
    vInitMainPWM (); // Initialise the main rotor PWM peripheral.
    vInitTailPWM (); // Initialise the tail rotor PWM peripheral.

    g_pRotorQueue = xQueueCreate(ROTOR_QUEUE_SIZE, ROTOR_ITEM_SIZE); // Create a queue to hold the duty cycle.

    // Initialisation is complete, so turn on the output.
    PWMOutputState(PWM_MAIN_BASE, PWM_MAIN_OUTBIT, true);
    PWMOutputState(PWM_TAIL_BASE, PWM_TAIL_OUTBIT, true);

    //
    // Create the rotor task to set the desired PWM.
    //
    if(xTaskCreate(RotorTask, (const portCHAR *)"Rotor",
                       ROTORTASKSTACKSIZE, NULL,  tskIDLE_PRIORITY + ROTORTASKPRIORITY, NULL) != pdTRUE)
    {
        return(1);
    }

    //
    // Success.
    //

    return(0);
}
