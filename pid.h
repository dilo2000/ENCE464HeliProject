/*
 * pid.h
 *
 *  Created on: 27/08/2021
 *      Author: josh
 */


#ifndef PID_H_
#define PID_H_

typedef struct
{
    uint8_t kp, ki;
    uint8_t accum, limit;
    int16_t error;
} pid_t;

void pid_init(pid_t*, uint8_t, uint8_t, uint8_t);
void pid_update(pid_t*, int16_t, uint16_t);
int16_t pid_get(pid_t*);


#endif /* PID_H_ */

