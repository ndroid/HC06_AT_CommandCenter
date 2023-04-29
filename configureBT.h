/*
 * HC-05/06 AT Command Center
 * 
 *  Description: Header file for simple HC05/06 AT configuration program. 
 * 
 *  Created on: 22-Apr, 2023
 *      Author: miller4@rose-hulman.edu
 *    Modified: 26-Apr, 2023
 */

#ifndef CONFIGUREBT_H
#define CONFIGUREBT_H

#define BAUD_LIST_CNT   8         // count of baud rate options
#define VERS3_MIN_BAUD  2         // index for firmware 3.x minimum baud rate (4800)
#define PARITY_LIST_CNT 3         // count of UART parity options
#define FIRM_UNKNOWN   -1         // index for unknown firmware 
#define FIRM_VERSION1   0         // index for firmware 1.x/2.x models
#define FIRM_VERSION3   1         // index for firmware 3.x models
#define MODEL_UNKNOWN  -1         // index for unknown HC-xx device model
#define MODEL_HC06      0         // index for HC-06 device models
#define MODEL_HC05      1         // index for HC-05 device models
#define ROLE_UNKNOWN   -1         // index for unknown device role
#define ROLE_SLAVE      0         // index for HC-05 devices in slave role
#define ROLE_MASTER     1         // index for HC-05 devices in master role
#define ROLE_SLAVE_LOOP 2         // index for HC-05 devices in slave-loop role

#define ENDLINE_NLCR    "\r\n"    // for firmware version 3
#define ENDLINE_NONE    ""        // for firmware version 1/2
#define STATUS_OK       "OK"
#define UART_CMD        "AT+UART="
#define BAUD_CMD        "AT+BAUD"
#define ROLE_CMD        "AT+ROLE="
#define ROLE_REQ        "AT+ROLE?\r\n"

// values for UART configuration
#define STOP1BIT        0
#define STOP2BIT        1
#define NOPARITY        0
#define ODDPARITY       1
#define EVENPARITY      2

#define CONFIG_DELAY    20      // delay for basic configuration changes
#define SHORT_DELAY     100     // brief delay constant for UI
#define MENU_DELAY      2000    // delay before returning to menu after fault
#define FW1_RESPONSE    550     // for firmware version 1/2
#define FW3_RESPONSE    40      // for firmware version 3
#define BITS_PER_CHAR   12      // UART frames - worst case: parity, 2 stop bits

// macros for determining if firmware of connected device is known
#define VERSION_KNOWN   (firmVersion != FIRM_UNKNOWN)
#define VERSION_UNKNOWN (firmVersion == FIRM_UNKNOWN)

// constant arrays for configuration and AT command construction
const unsigned long baudRateList[] = {1200, 2400, 4800, 9600, 19200, 
                                        38400, 57600, 115200};
const uint8_t parityList[] = {SERIAL_8N1, SERIAL_8O1, SERIAL_8E1};
const String parityType[] = {"None", "Odd", "Even"};
const String parityCmd[] = {"AT+PN", "AT+PO", "AT+PE"};
const String roleString[] = {"Slave", "Master", "Slave-Loop"};
const String lineEnding[] = {"", "\r\n"};
const String atCommands[][2] = {
                              {"AT", "AT\r\n"},
                              {"AT+VERSION", "AT+VERSION?\r\n"},
                              {"AT+NAME", "AT+NAME="},
                              {"AT+PIN", "AT+PSWD="}};
// indexes for AT commands within constant arrays
enum HC06commands {ECHO = 0,
                  HCVERSION,
                  BTNAME,
                  BTPIN,
                  UART_GET,
                  BAUD_SET,
                  PARITY_SET,
                  OTHER_CMD};

// Worst-case count of expected characters for response to commands.
//  Indexed based on HC06commands values.
const int responseChars[] = {
                            4,      // AT
                            26,     // AT+VERSION
                            22,     // AT+NAME
                            6,      // AT+PIN/AT+PSWD
                            22,     // AT+UART
                            8,      // AT+UART
                            8,      // AT+UART
                            40};    // other
// response times for AT commands by firmware version
const unsigned long responseMS[] = {FW1_RESPONSE, FW3_RESPONSE};

// global variables
int deviceModel = MODEL_UNKNOWN;
int firmVersion = FIRM_UNKNOWN;
int deviceRole = ROLE_UNKNOWN;
int baudRate = VERS3_MIN_BAUD;
int parity = NOPARITY;
int stopBits = STOP1BIT;
String hc06Version = "";
String nameBT, command;

#endif // CONFIGUREBT_H
