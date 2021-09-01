/*
 * File: yaw.c
 * Project: ENCE464 Assignment 1
 *
 * Authors:
 * - Oliver Dale
 * - Josh Roberts
 * - Micaela Cooper
 * - Angus Fairbairn
 *
 * Created on: 28.08.21
 *
 * Description: This module calculates the yaw angle of the helicopter. It is responsible for initialising
 * the associated GPIO pins and creating an ISR for the encoder edge count.
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
#include "driverlib/interrupt.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "yaw.h"
#include "priorities.h"

//*****************************************************************************
//
// Global variables for the yaw task.
//
//*****************************************************************************
static int16_t  g_i16Edges = 0;
static int16_t g_i16Angle = 0;

#define ENCODER_SLOTS           448 // Maximum slots in encoder.

//*****************************************************************************
//
// The stack size for the yaw handler task.
//
//*****************************************************************************
#define YAWTASKSTACKSIZE        128         // Stack size in words

//*****************************************************************************
//
// A counting semaphore to update the yaw from the ISR.
//
//*****************************************************************************
static SemaphoreHandle_t xBinarySemaphore;

//*****************************************************************************
//
// Yaw configuration details.
//
//*****************************************************************************
#define YAW_CHAN_PERIPH_GPIO       SYSCTL_PERIPH_GPIOB
#define YAW_CHAN_GPIO_BASE         GPIO_PORTB_BASE
#define YAW_CHAN_GPIO_STRENGTH     GPIO_STRENGTH_2MA
#define YAW_CHAN_A_GPIO_PIN        GPIO_PIN_0
#define YAW_CHAN_B_GPIO_PIN        GPIO_PIN_1
#define YAW_CHAN_PIN_TYPE          GPIO_PIN_TYPE_STD_WPD

#define YAW_REF_PERIPH_GPIO        SYSCTL_PERIPH_GPIOC
#define YAW_REF_GPIO_BASE          GPIO_PORTC_BASE
#define YAW_REF_GPIO_STRENGTH      GPIO_STRENGTH_2MA
#define YAW_REF_GPIO_PIN           GPIO_PIN_4
#define YAW_REF_PIN_TYPE           GPIO_PIN_TYPE_STD_WPU

//*****************************************************************************
//
// Local prototypes for the yaw module.
//
//*****************************************************************************
void vInitYawPins(void);
void vUpdateEdges (uint8_t, uint8_t);
void vCheckRef(uint8_t);
static void vCheckLimitCases(void);
void vEdge2Angle(void);
void vUpdateYaw (void);
static void vYawHandlingTask( void *pvParameters);
void vYawIntHandler (void);

//*****************************************************************************
//
// Configures the encoder pins and interrupt handler for Channel A and B.
// Channel A: PB0, Channel B: PB1, Reference: PC4.
//
//*****************************************************************************
void
vInitYawPins (void)
{
    SysCtlPeripheralEnable(YAW_CHAN_PERIPH_GPIO);
    SysCtlPeripheralEnable(YAW_REF_PERIPH_GPIO);

    // Register the handler for the yaw channel base into the vector table
    GPIOIntRegister (YAW_CHAN_GPIO_BASE, vYawIntHandler);

    // GPIO Inputs Set Up
    GPIOPinTypeGPIOInput(YAW_CHAN_GPIO_BASE, YAW_CHAN_A_GPIO_PIN);
    GPIOPadConfigSet (YAW_CHAN_GPIO_BASE, YAW_CHAN_A_GPIO_PIN, YAW_CHAN_GPIO_STRENGTH,
                      YAW_CHAN_PIN_TYPE);

    GPIOPinTypeGPIOInput(YAW_CHAN_GPIO_BASE, YAW_CHAN_B_GPIO_PIN);
    GPIOPadConfigSet (YAW_CHAN_GPIO_BASE, YAW_CHAN_B_GPIO_PIN, YAW_CHAN_GPIO_STRENGTH,
                      YAW_CHAN_PIN_TYPE);

    GPIOPinTypeGPIOInput(YAW_REF_GPIO_BASE, YAW_REF_GPIO_PIN);
    GPIOPadConfigSet (YAW_REF_GPIO_BASE, YAW_REF_GPIO_PIN, YAW_REF_GPIO_STRENGTH,
                      YAW_REF_PIN_TYPE);

    // Set up the pin change interrupt for yaw channels. (both edges)
    GPIOIntTypeSet (YAW_CHAN_GPIO_BASE, YAW_CHAN_A_GPIO_PIN, GPIO_BOTH_EDGES);
    GPIOIntTypeSet (YAW_CHAN_GPIO_BASE, YAW_CHAN_B_GPIO_PIN, GPIO_BOTH_EDGES);

    // Enable the pin change interrupt
    GPIOIntEnable (YAW_CHAN_GPIO_BASE, YAW_CHAN_A_GPIO_PIN | YAW_CHAN_B_GPIO_PIN);
    IntEnable (INT_GPIOB);  // NB: INT_GPIOB is defined in inc/hw_ints.h
}

//*****************************************************************************
//
// Getter function to get updated yaw angle.
//
//*****************************************************************************
int16_t
GetYawAngle (void)
{
    return g_i16Angle;
}

void
vUpdateEdges (uint8_t ui8YawA, uint8_t ui8PrevYawB)
{
    if (ui8YawA ^ ui8PrevYawB) {
        g_i16Edges--;
    } else {
        g_i16Edges++;
    }
}


void
vCheckRef(uint8_t ui8Ref)
{
    if (ui8Ref == 0) {
        g_i16Edges = 0;
    }
}

static void
vCheckLimitCases(void)
{
    if (g_i16Edges >= 450) {
        g_i16Edges = 0;
    } else if (g_i16Edges < 0) {
        g_i16Edges = 449;
    }
}

void
vEdge2Angle(void)
{
    g_i16Angle = (g_i16Edges * 360)/ENCODER_SLOTS;
}

//*****************************************************************************
//
// Updates the edge count based on the digital output of channel A and B
// and calculates the new yaw angle.
//
//*****************************************************************************
void
vUpdateYaw (void)
{
    uint8_t ui8YawA;
    uint8_t ui8YawB;

    static uint8_t ui8PrevYawB;

    uint8_t ui8Ref = 1;

    ui8Ref = GPIOPinRead(YAW_REF_GPIO_BASE, YAW_REF_GPIO_PIN);
    ui8YawA = GPIOPinRead(YAW_CHAN_GPIO_BASE, YAW_CHAN_A_GPIO_PIN);
    ui8YawB = GPIOPinRead(YAW_CHAN_GPIO_BASE, YAW_CHAN_B_GPIO_PIN);

    if (ui8YawB == 2) { ui8YawB = 1;}

    vUpdateEdges(ui8YawA, ui8PrevYawB);

    vCheckRef(ui8Ref);

    vCheckLimitCases();

    vEdge2Angle();

    ui8PrevYawB = ui8YawB;
}

//*****************************************************************************
//
// Runs update yaw when a semaphore is avaliable. Decrements semaphore each time.
//
//*****************************************************************************
static void
vYawHandlingTask( void *pvParameters )
{
    while(1)
    {
        xSemaphoreTake( xBinarySemaphore, portMAX_DELAY );
        vUpdateYaw();
    }
}

//*****************************************************************************
//
// Handler function for ISR triggered by encoder. Clears the ISR and then increments
// a counting semaphore to update the yaw.
//
//*****************************************************************************

void
vYawIntHandler (void)
{
     BaseType_t xHigherPriorityTaskWoken;

     GPIOIntClear (YAW_CHAN_GPIO_BASE, YAW_CHAN_A_GPIO_PIN | YAW_CHAN_B_GPIO_PIN);

     /* The xHigherPriorityTaskWoken parameter must be initialized to pdFALSE as it
     will get set to pdTRUE inside the interrupt safe API function if a context switch
     is required. */
     xHigherPriorityTaskWoken = pdFALSE;

     /* 'Give' the semaphore. Will unblock the deferred interrupt handling task. */
     xSemaphoreGiveFromISR( xBinarySemaphore, &xHigherPriorityTaskWoken );

     /*If xHigherPriorityTaskWoken was set to pdTRUE inside xSemaphoreGiveFromISR() then
     calling portYIELD_FROM_ISR() will request a context switch. */
     portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

//*****************************************************************************
//
// Initialises ReadAngle Task. Includes configuring yaw pins, counting
// semaphore and yaw handling task.
//
//*****************************************************************************

uint32_t
InitReadAngle (void)
{
    vInitYawPins();
    xBinarySemaphore = xSemaphoreCreateBinary();

    // Check semaphore was created successfully.
    if (xBinarySemaphore == NULL)
    {
        while (1)
        {
        }
    }

    if(xTaskCreate(vYawHandlingTask, (const portCHAR *)"YawHandlingTask",
                   YAWTASKSTACKSIZE, NULL,  tskIDLE_PRIORITY + YAWTASKPRIORITY,  NULL) != pdTRUE)
    {
        return(1);
    }

    return(0);
}

