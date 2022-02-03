# HC06_AT_CommandCenter
Arduino program for simplifying AT command configuration of HC-06 devices over UART.


 HC-06 AT Command Center
 
 Description: Simple HC06 configuration program. Requires 2nd UART (Serial1) defined. 
              Provides user menu for selecting configuration changes. Initial Serial1
              settings are 9600 8N1 (matches default HC-06 settings). Arduino device
              UART settings can be changed to match HC-06 through program. HC-06 must
              be in configuration mode (LED blinking to indicate Not Connected).
              
    HC06 connections:
      TXD <--> pin 3 (Serial 1 RX)
      RXD <--> pin 4 (Serial 1 TX)
      
  Created on: 18-Oct, 2021
      Author: miller4@rose-hulman.edu
    Modified: 30-Jan, 2022
    Revision: 1.2

