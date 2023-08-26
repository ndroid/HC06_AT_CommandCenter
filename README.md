# HC-05/06 AT Command Center
Arduino program for simplifying AT command configuration of HC-05/06 devices over UART.

Version 3.0

## Overview
  Description: Simple HC05/06 AT configuration program. Requires 2nd UART (Serial1) defined. 
 
   Provides user menu for selecting configuration changes. Automatically identifies
   device (HC-05 or HC-06), firmware version, baud and parity settings.
   Serial1 automatically configured to match HC-05/06 UART settings. HC-05/06 
   must be in configuration mode (AT mode) (LED blinking to indicate Not Connected).
   Serial monitor settings are 57600 8N1.
 
   Recent batches of HC-06 appear to have HC-05 firmware (reporting Version 3).
   There is no documentation of a Version 3 firmware for HC-06. AT commands differ
   for HC-05 firmware, including CR+NL command terminators. Support for this version
   has been added beginning with Revision 2 of this software.
 
 <dl>
   <dt> AT response delays:</dt>
   <dd> Around 10~25ms for Version 3.x  (newline terminated) - max observed 35ms<br>
   <dd> Around 500ms for Version 1.x    (timeout terminated) - max observed 525ms<br>
   <dd> Serial writes are asynchronous, so delays must also consider write time</dd>
 </dl>

 <br>
  <b>HC06 connections (for 5V boards - resistors not needed for 3V3):</b>

                TXD <----------------> [Serial 1 RX]
                RXD <----+---R_220---> [Serial 1 TX]
                         |
                         |
                       R_330
                         |
                         |
                        Vss
 
 <br>
  <b>HC05 connections: same as above, but also include (for AT mode selection)</b>

                CMD <----+---R_220---> [pin 10]
                         |
                         |
                       R_330
                         |
                         |
                        Vss
 
 <br>
  <b>Pin connections:</b>

                    board        Mega    MKR   Uno WiFi  Zero    Due    MSP432
      -------------------+-------------------------------------------------------
        [Serial 1 RX]    |        19      13      0        0      19       3 
        [Serial 1 TX]    |        18      14      1        1      18       4 

<br>

### History

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
        Modified: 25-Aug, 2023
        Revision: 3.0
                * added support and auto-detection for HC-05

### License
Up to version 1.4.0, the license is GPLv3.
From version 2.0.0, the license is the MIT license.

### Copyright
Copyright (c) 2021-2023 chris miller (https://github.com/ndroid)
