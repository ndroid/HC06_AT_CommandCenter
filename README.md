# HC06_AT_CommandCenter
Arduino program for simplifying AT command configuration of HC-06 devices over UART.


 HC-06 AT Command Center
 
 Description: Simple HC06 AT configuration program. Requires 2nd UART (Serial1) defined. 
              Provides user menu for selecting configuration changes. Attempsts to identify 
              HC-06 frame and baud settings (9600 8N1 default HC-06 settings), and firmware
              version. NL+CR required by new firmware, but not older firmware. Arduino device
              UART settings can be changed to match HC-06 through program. HC-06 must be 
              in configuration mode (AT mode) (LED blinking to indicate Not Connected).
              
    HC06 connections:
      TXD <--> pin 3 (Serial 1 RX)
      RXD <--> pin 4 (Serial 1 TX)
      
  Created on: 18-Oct, 2021
      Author: miller4@rose-hulman.edu
    Modified: 30-Jan, 2022
    Revision: 1.2
            * provides user menu for selecting desired configuration
    Modified: 7-Feb, 2022
    Revision: 1.3
            * updated end-line characters to support devices with newer
                firmware (3.0+) HC-05 firmware?
