# 36V-10A High Power Motor Driver

## About the project
This project was completed as a part of the fast prototyping EE challenge (To be completed in 3 days) carrying the following Design rules and consideration.

Design a bidirectional, variable speed / PWM controlled brushed DC motor controller.
* Include a microcontroller of your choice in your design.
* The only ICs you should include in your design is the microcontroller and a linear voltage regulator. All other components should be discrete components (passives, transistors, diodes, etc).
* The design will be powered by a 36V power supply.
* The design should be able to provide up to 10A to the motor (also rated for 36V).
* Submit a drawing of a schematic of your design, and include the part numbers of non-passive components.
* Additionally, submit a code snippet of a piece of firmware for the microcontroller that does the following:
* Ramp up the motor from 0% to 100% forward over 5 seconds.
* Ramp down the motor from 100% forward to 0% over 5 seconds.
* Ramp up the motor from 0% to 100% reverse over 5 seconds.
* Ramp down the motor from 100% reverse to 0% over 5 seconds.
* Please think through success criteria of above in a real-world scenario and adjust/optimize accordingly.

## Circuit

### H-Bridge Design
![H-Bridge](https://github.com/rhitvik/36V-10A_High_Power_Motor_Drawer/blob/master/Screenshot/H_Bridge.PNG)

### P-Channel 
MOSFET: AO4421 | MOSFET P-CH 60V 6.2A 8SOIC

Hence 2 used in parallel to increase the current sourcing capability to the motor. Also, using 2 MOSFETs in parallel reduces the heating effects as well as drain-source ESR resistance.
Zener diodes NZ9F10VT5G used to maintain Vgs = 10 V for minimum Drain-Source Resistance, each driven by BC846BLT3G transistor. This is where we apply the PWM pulse.

### N-Channel 
MOSFET: HUF76609D3ST | MOSFET N-CH 100V 10A DPAK
Used 1 to sink the rated current coming from the motor. Totem pole arrangement (MMBT3904 and MMBT3906) for driving this transistor used. Resistor configuration a max passage of 50 mA while driving the MOSFET with Vgs = 10 V for minimum Drain-Source Resistance.
MBR1560SRT Schottky diodes used for commutation-spike protection and freewheeling purposes.

### Reverse Polarity Protection
STD35P6LLF6, P-channel MOSFET employed for reverse polarity protection and Schottky BZT52C117F keeps its Rds minimized.

### Linear Regulator
BD750L5FP-CE2 IC REG LINEAR 5V 500MA TO252-3 to power the microcontroller.
Zener MMSZ5V1T1G to ensure that heavy spikes do not destroy the transistor gates/PWM  pins. 

## Suggested PCB Specifications
* Everything is SMD except for the terminal blocks. Space efficiency via SMD components whereas terminal blocks require a stronger connection. 
* 2 Oz Cu PCB recommended. Via Stitch for EMC/EMI considerations and recommended metal enclosures that can also act as heat sinks.

## Code
[Firmware Code (Developed over Attiny10)](https://github.com/rhitvik/36V-10A_High_Power_Motor_Drawer/blob/master/Code/main.cpp)

## Schematic Capture and Libraries
[Link to project](https://github.com/rhitvik/36V-10A_High_Power_Motor_Drawer/tree/master/Motor_Driver_Rev-2)

## Download Documentation
[Click Here](https://github.com/rhitvik/36V-10A_High_Power_Motor_Drawer/blob/master/Schematic%20and%20Report.DOCX)

## How the Firmware is written?
* The PWM is generated in the peripheral using fast non-inverted PWM mode. The microcontroller is at 8MHz and the Prescaler clocks it down to 1MHz in the timer module. The frequency of PWM is 100Hz and the duty cycle increase/decreases 1% every 50ms. 
* Interrupt based timer also keep a track of elapsed time. After ever 5 seconds, the state of the machine is changed and follows the sequence as directed while ensuring that there are no faults.
* The error rate is negligible as the compare registers are double-buffered and the Input capture register value is static.

