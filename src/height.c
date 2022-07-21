/*
 * File: height.c
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
 * Description: This module samples the ADC peripheral to get the height of the
 * helicopter. An interrupt is triggered at 10Hz, giving a semaphore to the ADC
 * handling task. A counting semaphore has been used to ensure no samples are
 * missed.
 *
 * Note: ADCIntHandler is adapted from "Master the FreeRTOS Real Time Kernel" by Richard Barry.
 *
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/adc.h"
#include "utils/ustdlib.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "height.h"
#include "priorities.h"
#include "debugger.h"

//*****************************************************************************
//
// Global variables for the height task.
//
//*****************************************************************************
uint32_t g_ui32Height;

//*****************************************************************************
//
// The stack of the height task.
//
//*****************************************************************************
#define PERIODICTASKSTACKSIZE    128
#define ADCTASKSTACKSIZE         128

//*****************************************************************************
//
// The delay of the ADC Interrupt Trigger.
//
//*****************************************************************************
#define ADC_DELAY               100

//*****************************************************************************
//
// The upper and lower limits for conversion of the height reading to percentage.
//
//*****************************************************************************
#define HEIGHT_LIMIT_UPPER               2210
#define HEIGHT_LIMIT_LOWER               160
#define HEIGHT_CONVERSION_MULTIPLIER     0.05

//*****************************************************************************
//
//Counting semaphore to process ADC readings.
//
//*****************************************************************************
static SemaphoreHandle_t xCountingSemaphore;

//*****************************************************************************
//
// Local prototypes for the height module.
//
//*****************************************************************************
static void vPeriodicADCTask (void *pvParameters);
static void vADCHandlingTask (void *pvParameters);
void ADCIntHandler( void );
void vInitADC(void);

//*****************************************************************************
//
// Converts height voltage reading to a percentage.
// NOTE: Currently altered to read height from Orbit BoosterPack potentiometer
//
//*****************************************************************************
static void
vGetPercentage(void)
{

    if (g_ui32Height >= HEIGHT_LIMIT_UPPER) {         // Check for upper limit
        g_ui32Height = 100;
    } else if (g_ui32Height < HEIGHT_LIMIT_LOWER) {    // Check for lower limit
        g_ui32Height = 0;
    } else {
        g_ui32Height = (g_ui32Height-HEIGHT_LIMIT_LOWER)*HEIGHT_CONVERSION_MULTIPLIER;
    }
}

//*****************************************************************************
//
// Getter function to get updated height percentage.
//
//*****************************************************************************
uint32_t
GetHeight(void)
{
    vGetPercentage();
    return g_ui32Height;
}

//*****************************************************************************
//
// A periodic task to trigger the ADC interrupt at 10Hz.
//
//*****************************************************************************
static void
vPeriodicADCTask (void *pvParameters)
{
    portTickType ui16DelayTime;
    ui16DelayTime = xTaskGetTickCount();

    while (1)
    {
        ADCProcessorTrigger(ADC0_BASE, 3);
        vTaskDelayUntil(&ui16DelayTime, ADC_DELAY / portTICK_RATE_MS);
    }
}

//*****************************************************************************
//
// Handling task for the ADC. Takes the semaphore and samples the ADC, storing
// the value in a global variable.
//
//*****************************************************************************
static void
vADCHandlingTask (void *pvParameters)
{

    while(1)
    {
        xSemaphoreTake( xCountingSemaphore, portMAX_DELAY );
        ADCSequenceDataGet(ADC0_BASE, 3, &g_ui32Height);      // Get the single sample from ADC0.
        SendToDebugger (g_ui32Height, HEIGHT);
    }
}

//*****************************************************************************
//
// Handles the ADC interrupt and clears it. Gives a semaphore to allow the ADC
// task to run.
//
//*****************************************************************************
void
ADCIntHandler( void )
{
     BaseType_t xHigherPriorityTaskWoken;

     ADCIntClear(ADC0_BASE, 3);

     /* The xHigherPriorityTaskWoken parameter must be initialized to pdFALSE as it
     will get set to pdTRUE inside the interrupt safe API function if a context switch
     is required. */
     xHigherPriorityTaskWoken = pdFALSE;

     /* 'Give' the semaphore. Will unblock the deferred interrupt handling task. */
     xSemaphoreGiveFromISR( xCountingSemaphore, &xHigherPriorityTaskWoken );

     /*If xHigherPriorityTaskWoken was set to pdTRUE inside xSemaphoreGiveFromISR() then
     calling portYIELD_FROM_ISR() will request a context switch. */
     portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

//*****************************************************************************
//
// Initializes the ADC peripheral and interrupt.
//
//*****************************************************************************
void
vInitADC(void)
{
        // NOTE: change ADC_CTL_CH9 to ADC_CTL_CH0 for sampling from emulator. Currently sampling from potentiometer

        SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);                             // The ADC0 peripheral must be enabled for configuration and use.

        ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);           // Enable sample sequence 3 with a processor signal trigger.

        ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH9 | ADC_CTL_IE |
                                 ADC_CTL_END);                                  // Configure step 0 on sequence 3.

        ADCSequenceEnable(ADC0_BASE, 3);                                        // Since sample sequence 3 is now configured, it must be enabled.

        ADCIntRegister(ADC0_BASE, 3, ADCIntHandler);

        ADCIntEnable(ADC0_BASE, 3);                                             // Enable interrupts for ADC0 sequence 3 (clears any outstanding interrupts)
}

//*****************************************************************************
//
// Initializes the ADC and periodic task.
//
//****************************************************************************
uint32_t
InitReadHeight (void)
{
    vInitADC (); // Initialise the ADC peripheral.

    // Create counting semaphore with max count of 10.
    xCountingSemaphore = xSemaphoreCreateCounting( 10, 0 );

    //
    // Create the a task to periodically trigger the ADC.
    //
    if(xTaskCreate(vPeriodicADCTask, (const portCHAR *)"PeriodicTask",
                   PERIODICTASKSTACKSIZE, NULL,  tskIDLE_PRIORITY + PERIODICTASKPRIORITY, NULL) != pdTRUE)
    {
        return(1);
    }

    //
    // Create the ADC handling task to read the ADC.
    //
    if(xTaskCreate(vADCHandlingTask, (const portCHAR *)"ADCRead",
                   ADCTASKSTACKSIZE, NULL,  tskIDLE_PRIORITY + ADCTASKPRIORITY, NULL) != pdTRUE)
    {
        return(1);
    }

    return(0);
}
