/**
 * HC-05 Configuration Example
 * 
 *  Description: Configure HC05 using simple user serial menu to manage AT 
 *              communications. Requires 2nd UART (Serial1) defined. Automatically 
 *              identifies device (HC-05 or HC-06), firmware version, baud and 
 *              parity settings. Serial1 automatically configured to match HC-05 
 *              UART settings. 
 * 
 *              HC-05 must be in configuration mode (AT mode) (LED blinking to 
 *              indicate Not Connected). 
 *              
 *                
 *    HC05 connections (for 5V boards - resistors not needed for 3V3):
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
 *    [additional connections for AT mode selection]
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
 *      Author: ndroid
 *    Modified: 26-Aug, 2023
 */

#include <configureBT.h>

#define MODE_PIN    10

void setup() {
  // configure Serial Monitor UART (57600 8N1)
  Serial.begin(57600);
  delay(100);
}

void loop() {
  // Will display user menu to Serial and handle user selection
  commandMenu();
}
