/**
 * @file configureBT.h
 * 
 * HC-05/06 AT Command Center
 *  Version 3.0
 * 
 *  Description: Simple HC05/06 AT configuration program. Requires 2nd UART (Serial1) defined. 
 * 
 *              Provides user menu for selecting configuration changes. Automatically identifies
 *              device (HC-05 or HC-06), firmware version, baud and parity settings.
 *              Serial1 automatically configured to match HC-05/06 UART settings. HC-05/06 
 *              must be in configuration mode (AT mode) (LED blinking to indicate Not Connected).
 *              Serial monitor settings are 57600 8N1.
 *              
 *              Recent batches of HC-06 appear to have HC-05 firmware (reporting Version 3).
 *              There is no documentation of a Version 3 firmware for HC-06. AT commands differ
 *              for HC-05 firmware, including CR+NL command terminators. Support for this version
 *              has been added beginning with Revision 2 of this software.
 *              
 *              AT response delays:
 *                Around 10~25ms for Version 3.x  (newline terminated) - max observed 35ms
 *                Around 500ms for Version 1.x    (timeout terminated) - max observed 525ms
 *                Serial writes are asynchronous, so delays must also consider write time
 *                
 *    HC06 connections (for 5V boards - resistors not needed for 3V3):
 * 
 *                TXD -----------------> [Serial 1 RX]
 *                RXD <----+---R_220---- [Serial 1 TX]
 *                         |
 *                         |
 *                       R_330
 *                         |
 *                         |
 *                        Vss
 * 
 *    HC05 connections: same as above, but also include (for AT mode selection)
 * 
 *                STATE  -----------------> [State pin]
 *                EN/KEY <----+---R_220---- [Mode pin]
 *                            |
 *                            |
 *                          R_330
 *                            |
 *                            |
 *                           Vss
 * 
 *    Pin connections:
 *                    board        Mega    MKR   Uno WiFi  Zero    Due    MSP432
 *      -------------------+-------------------------------------------------------
 *        [Serial 1 RX]    |        19      13      0        0      19       3 
 *        [Serial 1 TX]    |        18      14      1        1      18       4 
 *      
 * 
 *  Created on: 18-Oct, 2021
 *      Author: miller4@rose-hulman.edu
 *    Modified: 25-Aug, 2023
 *    Revision: 3.0
 */

#ifndef CONFIGUREBT_H
#define CONFIGUREBT_H

#define ROLE_UNKNOWN   -1         // index for unknown device role
#define ROLE_SLAVE      0         // index for HC-05 devices in slave role
#define ROLE_MASTER     1         // index for HC-05 devices in master role
#define ROLE_SLAVE_LOOP 2         // index for HC-05 devices in slave-loop role

/**
 * commandMenu
 *
 * @brief Print user menu to Serial and handle selection.
 */
void commandMenu();

/**
 * scanDevice
 *  
 * Automated scan of Bluetooth module to determine configuration of UART.
 * Will identify version of firmware, baud rate, and parity setting, and set
 * Serial1 to match HC-xx UART settings.
 * 
 * @returns true if UART configuration successfully identified
 */
bool scanDevice();

/**
 * testEcho
 *  
 * Send AT command to test configuration of UART.
 * If OK response not received, firmVersion set to FIRM_UNKNOWN.
 */
void testEcho();

/**
 * getRole
 *  
 * Send AT command to request current BT role for HC-05 device.
 * 
 * @returns  current role setting of device:
 *    - ROLE_UNKNOWN  - unknown (may be HC-06 fw version 1.x)
 *    - ROLE_SLAVE    - acts as discoverable wireless UART device ready for transparent data exchange
 *    - ROLE_MASTER   - scans for a remote bluetooth (slave) device, pairs, and setup connection
 *    - ROLE_SLAVE_LOOP - data loop-back Rx-Tx, used mainly for testing
 */
int getRole();

/**
 * setRole
 *  
 * Send AT command to set BT role of HC-05 device.
 * 
 * @returns true if request succeeds.
 */
bool setRole(unsigned int role);

#endif // CONFIGUREBT_H
