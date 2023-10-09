# HC-05/06 AT Command Center
Arduino library for simplifying AT command configuration of HC-05/06 devices over UART.

Version 2.0

## Overview
  <b>Description</b>: Simple HC05/06 AT configuration library for Arduino. 
  Requires 2nd UART (Serial1) defined. Full repo can be found at 
  [HC06_AT_CommandCenter](https://github.com/ndroid/HC06_AT_CommandCenter). 
  Reference site for this library can be found 
  [here](https://ndroid.github.io/HC06_AT_CommandCenter/index.html).
 
   Provides user menu for selecting configuration changes. Automatically identifies
   device (HC-05 or HC-06), firmware version, baud and parity settings.
   Serial1 automatically configured to match HC-05/06 UART settings. HC-05/06 
   must be in configuration mode (AT mode) (LED blinking to indicate Not Connected).
 
 <br>
  <b>HC06 connections (for 5V boards - resistors not needed for 3V3):</b>

                TXD -----------------> [Serial1 RX]
                RXD <----+---R_1K----- [Serial1 TX]
                         |
                         |
                       R_2K
                         |
                        \|/
                        Vss
 
 <br>
  <b>HC05 connections: same as above, but also include (for AT mode selection):</b>

                STATE  -----------------> [State pin]
                EN/KEY <----+---R_1K----- [Mode pin]
                            |
                            |
                          R_2K
                            |
                           \|/
                           Vss
 
 <br>
 <dl>
   <dt> <b>Command mode</b> (AT mode): </dt>
   <dd> Device LED should be blinking fast (> 2 Hz) when in command (AT) mode.<br>
   <dd> When LED is solid, device is paired and connected.<br>
   <dd> If LED is blinking slowly (HC-05), device is in fixed AT mode. In this 
    mode, UART baud rate is fixed at 38400 with no parity. This occurs 
    when EN/KEY pin is tied HIGH while power is connected. For this reason, 
    [Mode pin] will be set to INPUT mode when not communicating AT commands. If
    LED is blinking slowly, try disconnecting and reconnecting power to HC-05
    device.</dd>
 </dl>


 <br>
  <b>Pin connections:</b>

                    board        Mega    MKR   Uno WiFi  Zero    Due    MSP432
      -------------------+-------------------------------------------------------
        [Serial1 RX]     |        19      13      0        0      19       3 
        [Serial1 TX]     |        18      14      1        1      18       4 

<br>

   Recent batches of HC-06 appear to have HC-05 firmware (reporting Version 3).
   There is no documentation of a Version 3 firmware for HC-06. AT commands differ
   for HC-05 firmware, including CR+NL command terminators. Support for this version
   has been added beginning with Revision 1 of this software.
 
 <dl>
   <dt> AT response delays:</dt>
   <dd> Around 10~25ms for Version 3.x  (newline terminated) - max observed 35ms<br>
   <dd> Around 500ms for Version 1.x    (timeout terminated) - max observed 525ms<br>
   <dd> Serial writes are asynchronous, so delays must also consider write time</dd>
 </dl>

### History

      Created on: 18-Oct, 2021
      Author: ndroid (miller4@rose-hulman.edu)
    
        Modified: 09-Oct, 2023
        Revision: 2.0.1
                * corrects UART failure following detectDevice() failure (Issue #6)
        Modified: 25-Sep, 2023
        Revision: 2.0
                * added support and auto-detection for HC-05
                * defined classes for interfacing HC-0x devices
        Modified: 22-Apr, 2023
        Revision: 1.0
                * added support and auto-detection for firmware versions
                    1.x and 3.x
        Modified: 24-Apr, 2022
        Revision: 0.4
                * added option to auto-detect firmware version and UART 
                    configuration
        Modified: 7-Feb, 2022
        Revision: 0.3
                * updated end-line characters to support devices with newer
                    firmware (3.0+) HC-05 firmware?
        Modified: 30-Jan, 2022
        Revision: 0.2
                * provides user menu for selecting desired configuration

### License
Up to version 0.4.0, the license is GPLv3.
From version 1.0.0, the license is the MIT license.

### Copyright
Copyright (c) 2021-2023 chris miller (https://github.com/ndroid)
