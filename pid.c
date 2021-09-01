/*
 * pid.c
 *
 *  Created on: Aug 27, 2021
 *      Author: josh
 */

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"

#include "pid.h"

int16_t
pid_get(pid_t *pid)
{
    return pid->kp * pid->error + pid->ki * pid->accum;
}

void
pid_update(pid_t *pid, int16_t error, uint16_t dt)
{

    pid->accum += error * dt;

    if (pid->accum > pid->limit)
    {
        pid->accum = pid->limit;
    }
    if (pid->accum < -pid->limit)
    {
        pid->accum = -pid->limit;
    }

    pid->error = error;
}

void pid_init(pid_t *pid, uint8_t kp, uint8_t ki, uint8_t limit)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->limit = limit;
    pid->accum = 0;
    pid->error = 0;
}
