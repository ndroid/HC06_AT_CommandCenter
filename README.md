# HC06_AT_CommandCenter
Arduino program for simplifying AT command configuration of HC-06 devices over UART.

Version 2.0

## Overview
  Description: Simple HC05/06 AT configuration program. Requires 2nd UART (Serial1) defined. 
 
              Provides user menu for selecting configuration changes. Attempts to identify 
              HC-05/06 frame and baud settings (9600 8N1 default HC-06 settings), and firmware
              version. Serial1 automatically configured to match HC-06 UART settings. HC-05/06 
              must be in configuration mode (AT mode) (LED blinking to indicate Not Connected).
              Serial monitor settings are 57600 8N1.
              
              Recent batches of HC-06 appear to have HC-05 firmware (reporting Version 3).
              There is no documentation of a Version 3 firmware for HC-06. AT commands differ
              for HC-05 firmware, including CR+NL command terminators. Support for this version
              has been added beginning with Revision 2 of this software.
              
              AT response delays:
                Around 10~25ms for Version 3.x  (newline terminated) - max observed 35ms
                Around 500ms for Version 1.x    (timeout terminated) - max observed 525ms
                Serial writes are asynchronous, so delays must also consider write time
                
    HC06 connections:
                              pin/board   Mega    MKR   Uno WiFi  Zero    Due
      TXD <--> [Serial 1 RX]               19      13      0        0      19
      RXD <--> [Serial 1 TX]               18      14      1        1      18
      
  Created on: 18-Oct, 2021
  Author: miller4@rose-hulman.edu
    
    Modified: 30-Jan, 2022
      Revision: 1.2
            * provides user menu for selecting desired configuration
    Modified: 7-Feb, 2022
      Revision: 1.3
            * updated end-line characters to support devices with newer
                firmware (3.0+) HC-05 firmware?
    Modified: 24-Apr, 2022
      Revision: 1.4
            * added option to auto-detect firmware version and UART 
                configuration
    Modified: 22-Apr, 2023
      Revision: 2.0
            * added support and auto-detection for firmware versions
                1.x and 3.x

## License
Up to version 1.4.0, the license is GPLv3.
From version 2.0.0, the license is the MIT license.

## Copyright
Copyright (c) 2021-2023 chris miller (https://github.com/ndroid)
