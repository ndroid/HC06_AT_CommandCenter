/**
 * HC-06 Configuration Example
 * 
 *  Description: Configure HC-06 using simple user serial menu to manage AT 
 *              communications. Requires 2nd UART (Serial1) defined. Automatically 
 *              identifies device (HC-05 or HC-06), firmware version, baud and 
 *              parity settings. Serial1 automatically configured to match HC-06 
 *              UART settings. 
 * 
 *              HC-06 must be in configuration mode (AT mode) (LED blinking to 
 *              indicate Not Connected). 
 *              
 *                
 *    HC06 connections (for 5V boards - resistors not needed for 3V3):
 * 
 *                TXD -----------------> [Serial1 RX]
 *                RXD <----+---R_1K----- [Serial1 TX]
 *                         |
 *                         |
 *                       R_2K
 *                         |
 *                        \|/
 *                        Vss
 * 
 *  Command mode (AT mode): 
 *   - Device LED should be blinking fast (> 2 Hz) when in command (AT) mode.
 *   - When LED is solid, device is paired and connected.
 * 
 *    Pin connections:
 *                    board        Mega    MKR   Uno WiFi  Zero    Due    MSP432
 *      -------------------+-------------------------------------------------------
 *        [Serial1 RX]    |        19      13      0        0      19       3 
 *        [Serial1 TX]    |        18      14      1        1      18       4 
 *      
 * 
 *      Author: ndroid
 *    Modified: 26-Aug, 2023
 */

#include <configureBT.h>

HCBT hc06;

void setup() {
  // configure Serial Monitor UART (57600 8N1)
  Serial.begin(57600);
  delay(1000);
  Serial.println("Will scan to automatically configure UART for connected device.");
  Serial.println("Enter any character when ready to scan.");
  while (Serial.available() < 1);
  delay(100);
  Serial.readString();   // clear buffer
  if (!hc06.detectDevice(true)) {
    Serial.println("Device not identified!");
    Serial.println("Check connections and restart to scan again.");
  }
}

void loop() {
  // Will display user menu to Serial and handle user selection
  hc06.commandMenu();
  Serial.println("\nEnter any character to return to menu.");
  while (Serial.available() < 1);
  delay(100);
  Serial.readString();   // clear buffer
}
