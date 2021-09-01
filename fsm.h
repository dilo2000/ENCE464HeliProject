/*
 * fsm.h
 *
 *  Created on: 29/08/2021
 *      Author: josh
 */


#ifndef FSM_H_
#define FSM_H_

typedef enum {
    IDLE,
    TAKEOFF,
    FLYING,
    LANDING
} State;

void fsm_update();

#endif /* FSM_H_ */

