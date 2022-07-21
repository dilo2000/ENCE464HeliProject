### Helicopter Controller

This repository contains the source files for a helicopter application. The application controls an emulated helicopter using a PI controller for use on a Tiva launchpad. The height and yaw of the helicopter can be set using the left, right, up and down button presses. The current status of the helicopter is displayed on the OLED BoosterPack. This program has been implemented using the FreeRTOS architecture. Each task is initialised in the main.c script and the scheduler is started. 

 The application contains the following tasks:
 
- **Display**: Displays the current position and desired position of the helicopter on the OLED display.

- **Rotor**: Drives the PWM to the tail and main rotor of the helicopter.
 
- **Buttons**: Records the button presses, updating the desired yaw and height of the helicopter in response.

- **Controller**: Controls the state machine of the helicopter and calculates the tail rotor duty cycle using a PI controller.

- **Height**: Reads the height of the helicopter from the ADC using periodically triggered interrupts.
 
 - **Angle**: Reads the yaw of the helicopter. ISR's are triggered at each edge change by the rotary encoder.

- **Debug**: Takes information from the controller, height and angle tasks to print to the UART via a FreeRTOS queue.