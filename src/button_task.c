/*
 * File: button_task.c
 * Project: ENCE464 Assignment 1
 *
 * Authors:
 * - Oliver Dale
 * - Josh Roberts
 * - Micaela Cooper
 * - Angus Fairbairn
 *
 *
 * Created on: 28.08.21
 *
 * Description: This module is responsible for reading the up, down, left and right buttons. The
 * reference values for the height and yaw are updated in response.
 *
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "utils/ustdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "stdio.h"

#include "buttons4.h"
#include "button_task.h"
#include "debugger.h"
#include "priorities.h"

//*****************************************************************************
//
// Global variables for the buttons task.
//
//*****************************************************************************
static int16_t g_i16RefAngle = 0;
static int16_t g_i16RefHeight = 0;
static uint8_t g_ui8ButPushed = 0;

//*****************************************************************************
//
// The stack size for the button task.
//
//*****************************************************************************
#define BUTTONTASKSTACKSIZE        128         // Stack size in words

//*****************************************************************************
//
// Local prototypes for the buttons module.
//
//*****************************************************************************
static void ButtonTask(void *pvParameters);
static void vCheckYawLimitCases (void);
static void vCheckHeightLimitCases (void);
uint8_t CheckYawButtons (void);
uint8_t CheckHeightButtons (void);
static void SetRefYaw (int16_t value);
static void SetRefHeight (int16_t value);
static void SetButPushed (uint8_t);


int16_t
GetRefYaw (void)
{
    return g_i16RefAngle;
}

static void
SetRefYaw (int16_t value)
{
    g_i16RefAngle = value;
}

int16_t
GetRefHeight (void)
{
    return g_i16RefHeight;
}

static void
SetRefHeight (int16_t value)
{
    g_i16RefHeight = value;
}

uint8_t
GetButPushed (void)
{
    return g_ui8ButPushed;
}

static void
SetButPushed (uint8_t value)
{
    g_ui8ButPushed = value;
}


//*****************************************************************************
//
// Setting limits for yaw and height
//
//*****************************************************************************
static void
vCheckYawLimitCases (void)
{
    if (g_i16RefAngle >= 360)
    {
        SetRefYaw(0);
    }
    else if (g_i16RefAngle < 0)
    {
        SetRefYaw(350);
    }
}

static void
vCheckHeightLimitCases (void)
{
    if (g_i16RefHeight > 100)
    {
        SetRefHeight(100);
    }
    else if (g_i16RefHeight < 0)
    {
        SetRefHeight(0);
    }
}

//*****************************************************************************
//
// Set the yaw using the left and right button
//
//*****************************************************************************
uint8_t
CheckYawButtons (void)
{
    uint8_t leftButState, rightButState;
    uint8_t butPushed = 0;

    leftButState = checkButton (LEFT);
    rightButState = checkButton (RIGHT);

    if (leftButState == PUSHED)
    {
        SetRefYaw (GetRefYaw() - 10);
        butPushed = 1;
    }
    else if (rightButState == PUSHED)
    {
        SetRefYaw (GetRefYaw() + 10);
        butPushed = 1;
    }
    return (butPushed);
}

//*****************************************************************************
//
// Set the height using the up and down buttons
//
//*****************************************************************************
uint8_t
CheckHeightButtons (void)
{
    uint8_t upButState, downButState;
    uint8_t butPushed = 0;

    upButState = checkButton (UP);
    downButState = checkButton (DOWN);

    if (upButState == PUSHED)
    {
        SetRefHeight(GetRefHeight() + 10);
        butPushed = 1;
    }
    else if (downButState == PUSHED)
    {
        SetRefHeight(GetRefHeight() - 10);
        butPushed = 1;
    }

    return (butPushed);
}


//*****************************************************************************
//
// Check for button inputs and update reference height and yaw
//
//*****************************************************************************
static void
ButtonTask (void *pvParameters)
{
    uint32_t ui32ButtonsDelay = 25;   //task operates every 25 ms

    portTickType ui16DelayTime;
    ui16DelayTime = xTaskGetTickCount();

    uint8_t butPushed, butPushed2 = 0;

    while (1)
    {
        //
        // Poll the debounced state of the buttons.
        //
        updateButtons();
        butPushed = CheckYawButtons();
        vCheckYawLimitCases();
        SendToDebugger (GetRefYaw(), YAWREF);

        updateButtons();
        butPushed2 = CheckHeightButtons();
        vCheckHeightLimitCases();
        SendToDebugger (GetRefHeight(), HEIGHTREF);

        if (butPushed || butPushed2)
        {
            SetButPushed (1);
        }
        else
        {
            SetButPushed (0);
        }

        vTaskDelayUntil(&ui16DelayTime, ui32ButtonsDelay / portTICK_RATE_MS);
    }
}

//*****************************************************************************
//
// Initializes the Button task.
//
//*****************************************************************************
uint32_t
InitButtonTask (void)
{
    initButtons();

    //
    // Create the buttons task.
    //
    if(xTaskCreate(ButtonTask, (const portCHAR *)"ButtonTask",
                   BUTTONTASKSTACKSIZE, NULL, tskIDLE_PRIORITY + BUTTONTASKPRIORITY, NULL) != pdTRUE)

    {
        return(1);
    }

    return(0);
}
