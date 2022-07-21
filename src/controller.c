/*
 * File: controller.c
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
 * Description: This module calculates the desired control signals for the helicopter. Uses pid.c and fsm.c.
 *
 *
 */

#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "controller.h"
#include "yaw.h"
#include "display.h"
#include "pid.h"
#include "button_task.h"
#include "rotor.h"
#include "fsm.h"
#include "debugger.h"
#include "priorities.h"

//*****************************************************************************
//
// The stack size for the button task.
//
//*****************************************************************************
#define CONTROLLERTASKSTACKSIZE        128         // Stack size in words

//*****************************************************************************
//
// Creates instance of PI struct for control of tail rotor.
//
//*****************************************************************************
static PI tail;

//*****************************************************************************
//
// Local prototypes for the controller module.
//
//*****************************************************************************


//*****************************************************************************
//
// FreeRTOS task. Updates fsm state.
//
//*****************************************************************************
static void
ControllerTask (void *pvParameters)
{
    /*PI Controller for Helirig emulator. Calculates duty for tail rotor.*/


    portTickType ui16DelayTime;
    uint32_t ui32ControllerDelay = 25;  //task operates every 25 ms

    ui16DelayTime = xTaskGetTickCount();

    while(1)
    {

        fsm_update();

        //
        // Wait for the required amount of time to check back.
        //
        vTaskDelayUntil(&ui16DelayTime, ui32ControllerDelay / portTICK_RATE_MS);
    }
}

//*****************************************************************************
//
// Initialises the Controller task.
//
//*****************************************************************************
uint32_t
InitControllerTask(void)
{

    vControlInit();

    //
    // Create the controller task.
    //
    if(xTaskCreate(ControllerTask, (const portCHAR *)"Controller",
                   CONTROLLERTASKSTACKSIZE, NULL, tskIDLE_PRIORITY + CONTROLLERTASKPRIORITY, NULL) != pdTRUE)

    {
        return(1);
    }

    //
    // Success.
    //

    return(0);
}

//*****************************************************************************
//
// Initialises the PI struct for the tail rotor control with desired PI
// characteristics.
//
//*****************************************************************************
void
vControlInit(void)
{
    uint8_t kp = 5;
    uint8_t ki = 1/1000;
    uint8_t limit = 5;
    pi_init(&tail, kp, ki, limit);
}

//*****************************************************************************
//
// Called by fsm.c. Returns duty for tail rotor given an error.
//
//*****************************************************************************
void
vControlUpdate(int16_t error)
{
    /*Updates PI Controller for Helirig emulator.*/

    portTickType ui16LastTime;
    portTickType ui16CurTime = 0;

    uint16_t dt;

    //
    // Get the current tick count.
    //
    ui16LastTime = ui16CurTime;
    ui16CurTime = xTaskGetTickCount();
    dt = 1000*(ui16CurTime - ui16LastTime) / configTICK_RATE_HZ; //in ms

    pi_update(&tail, error, dt);
}

//*****************************************************************************
//
// Called by fsm.c. Returns yaw error given a current yaw and a reference yaw.
//
//*****************************************************************************
int16_t
i16GetError(uint16_t ref, int16_t cur)
{
    int16_t error;
    error = ref - cur;

    if (error > 180)
    {
        error = error - 360;
    }
    else if (error < -180)
    {
        error = 360 + error;
    }

    return(error);
}


//*****************************************************************************
//
// Returns duty for the tail rotor using a PI controller.
//
//*****************************************************************************
uint16_t
ui16ControlGet(void)
{
    //
    // initialise limits
    //
    uint16_t dutyOffset = 74;
    uint16_t dutyMax = 95;
    uint16_t dutyMin = dutyOffset/2;
    uint16_t duty;

    duty = pi_get(&tail);
    duty += dutyOffset;

    if (duty >= dutyMax)
    {
        duty = dutyMax;
    }
    else if (duty <= dutyMin)
    {
        duty = dutyMin;
    }

    SendToDebugger (duty, DUTY);

    return(duty);
}
