 /*
 * File: display.c
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
 * Description: This module is responsible for displaying the current yaw and
 * height to the OLED display. The reference yaw and height are also displayed.
 *
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "utils/ustdlib.h"

#include "OrbitOLED/OrbitOLEDInterface.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "display.h"
#include "button_task.h"
#include "yaw.h"
#include "height.h"
#include "debugger.h"
#include "priorities.h"

//*****************************************************************************
//
// The stack, priority and delay of the display task.
//
//*****************************************************************************
#define DISPLAYTASKSTACKSIZE        128         // Stack size in words
#define DISPLAY_DELAY               100          // 100ms delay

//*****************************************************************************
//
// Local prototypes for the Display task.
//
//*****************************************************************************
void initDisplay (void);
static void DisplayTask(void *pvParameters);

//*****************************************************************************
//
// Intialise the Orbit OLED display
//
//*****************************************************************************
void
initDisplay (void)
{
    OLEDInitialise ();
}

//*****************************************************************************
//
// Displays the current yaw and height as well as the reference values.
//
//*****************************************************************************
static void
DisplayTask(void *pvParameters)
{
   char CurString[17];  // 16 characters across the display
   char RefString[17];

   uint16_t ui16RefYaw;
   uint16_t ui16RefHeight;
   int16_t i16Yaw;
   uint32_t ui32Height;

   ui16RefYaw = 0;

   portTickType ui16DelayTime;
   ui16DelayTime = xTaskGetTickCount();

   OLEDStringDraw ("Heli Monitor", 0, 0);

   while(1)
   {
       ui16RefYaw = GetRefYaw();
       ui16RefHeight = GetRefHeight();
       ui32Height = GetHeight();
       i16Yaw = GetYawAngle();

       usnprintf (RefString, sizeof(RefString), "YAW:%3d YAWR:%3d", i16Yaw, ui16RefYaw); // Display the current and reference yaw.
       OLEDStringDraw (RefString, 0, 2);
       usnprintf (CurString, sizeof(CurString), "ALT:%3d ALTR:%3d", ui32Height, ui16RefHeight);  // Display the current and reference height.
       OLEDStringDraw (CurString, 0, 3);

       //
       // Update the display at 10Hz.
       //
       vTaskDelayUntil(&ui16DelayTime, DISPLAY_DELAY / portTICK_RATE_MS);
   }
}

//*****************************************************************************
//
// Initialises the Display task.
//
//*****************************************************************************
uint32_t
InitDisplayTask (void)
{
    initDisplay (); // Initialise the OLED display.

    //
    // Create the display task.
    //
    if(xTaskCreate(DisplayTask, (const portCHAR *)"Display",
                       DISPLAYTASKSTACKSIZE, NULL,  tskIDLE_PRIORITY + DISPLAYTASKPRIORITY, NULL) != pdTRUE)
    {
        return(1);
    }

    return(0);
}
