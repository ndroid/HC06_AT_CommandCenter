/**
 * @file configureBT.h
 * 
 *  Description: Constants defined for HC-05/06 AT Command Center. 
 * 
 *  Created on: 25-Aug, 2023
 *      Author: miller4@rose-hulman.edu
 *    Modified: 26-Aug, 2023
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

#define BAUD_LIST_CNT   8         // count of baud rate options
#define VERS2_MIN_BAUD  2         // index for firmware 2.x/3.x minimum baud rate (4800)
#define PARITY_LIST_CNT 3         // count of UART parity options
#define FIRM_UNKNOWN    0         // index for unknown firmware 
#define FIRM_VERSION1   1         // index for firmware 1.x models
#define FIRM_VERSION2   2         // index for firmware 2.x/3.x models
#define MODEL_UNKNOWN   0         // index for unknown HC-xx device model
#define MODEL_HC06      1         // index for HC-06 device models
#define MODEL_HC05      2         // index for HC-05 device models

#define ENDLINE_NLCR    "\r\n"    // for firmware version 2/3
#define ENDLINE_NONE    ""        // for firmware version 1
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
#define FW1_RESPONSE    550     // for firmware version 1
#define FW2_RESPONSE    40      // for firmware version 2/3
#define BITS_PER_CHAR   12      // UART frames - worst case: parity, 2 stop bits

// macros for determining if firmware of connected device is known
#define VERSION_KNOWN   (firmVersion != FIRM_UNKNOWN)
#define VERSION_UNKNOWN (firmVersion == FIRM_UNKNOWN)

// constant arrays for configuration and AT command construction
const unsigned long baudRateList[] = {1200, 2400, 4800, 9600, 19200, 
                                        38400, 57600, 115200};
const uint32_t parityList[] = {SERIAL_8N1, SERIAL_8O1, SERIAL_8E1};
const String parityType[] = {"None", "Odd", "Even"};
const String parityCmd[] =  {"AT+PN", "AT+PO", "AT+PE"};
const String roleString[] = {"Secondary", "Primary", "Secondary-Loop"};
const String lineEnding[] = {"", "", "\r\n"};
const String requestVal[] = {"", "", "?\r\n"};
const String setValue[] =   {"", "", "="};
const String namePrefix[] = {"HCxx_", "HC06_", "HC05_"};
const String responsePrefix[] = {"[HC0x]: ", "[HC06]: ", "[HC05]: "};
const String atCommands[] = { "AT", 
                              "AT+VERSION", 
                              "AT+NAME", 
                              "AT+PIN",
                              "AT+PSWD"};

// indexes for AT commands within constant arrays
enum HCxxCommands {ECHO = 0,
                  HCVERSION,
                  BTNAME,
                  BTPIN,
                  BTPSWD,
                  UART_GET,
                  BAUD_SET,
                  PARITY_SET,
                  OTHER_CMD};

// Worst-case count of expected characters for response to commands.
//  Indexed based on HCxxCommands values.
const int responseChars[] = {
                            4,      // AT
                            26,     // AT+VERSION
                            22,     // AT+NAME
                            6,      // AT+PIN/AT+PSWD
                            6,      // AT+PIN/AT+PSWD
                            22,     // AT+UART
                            8,      // AT+UART
                            8,      // AT+UART
                            40};    // other

// response times for AT commands by firmware version
const unsigned long responseMS[] = {FW1_RESPONSE, FW1_RESPONSE, FW2_RESPONSE};

#define HC06_MENUSIZE     9

// string constants for HC-06 comman menu
//  index 0 not used because parseInt will return 0 for non-numeric entries
const String hc06Menu[] = { "", 
                      ") Set HC06 Baud Rate",                             // 1
                      ") Set HC06 BT name",                               // 2
                      ") Set HC06 BT pin",                                // 3
                      ") Set HC06 parity",                                // 4
                      ") Set local Baud Rate (for testing only)",         // 5
                      ") Set local parity (for testing only)",            // 6
                      ") Get version (useful to verify connection/baud)", // 7
                      ") Rescan HC06 device"};                            // 8


/*****************************************************************************
 * HC-05 specific constants
 * 
 * HC-05 modes of operation:
 *   Data mode -     CMD pin (EN/KEY) pulled to logic low level or unconnected
 *   Command mode -  CMD pin (EN/KEY) pulled to logic high level
 * 
 *****************************************************************************/

//#define CMD_PIN         8           // Arduino pin connected to CMD pin of HC-05
#define MODE_DATA       LOW         // HC-05 in data mode
#define MODE_COMMAND    HIGH        // HC-05 in command mode

const String errorCodes[] = {
                "0 Command Error/Invalid Command",
                "1 Results in default value",
                "2 PSKEY write error",
                "3 Device name is too long (>32 characters)",
                "4 No device name specified (0 lenght)",
                "5 Bluetooth address NAP is too long",
                "6 Bluetooth address UAP is too long",
                "7 Bluetooth address LAP is too long",
                "8 PIO map not specified (0 lenght)",
                "9 Invalid PIO port Number entered",
                "A Device Class not specified (0 lenght)",
                "B Device Class too long",
                "C Inquire Access Code not Specified (0 lenght)",
                "D Inquire Access Code too long",
                "E Invalid Iquire Access Code entered",
                "F Pairing Password not specified (0 lenght)",
                "10 Pairing Password too long (> 16 characters)",
                "11 Invalid Role entered",
                "12 Invalid Baud Rate entered",
                "13 Invalid Stop Bit entered",
                "14 Invalid Parity Bit entered",
                "15 No device in the Pairing List",
                "16 SPP not initialized",
                "17 SPP already initialized",
                "18 Invalid Inquiry Mode",
                "19 Inquiry Timeout occured",
                "1A Invalid/zero lenght address entered",
                "1B Invalid Security Mode entered",
                "1C Invalid Encryption Mode entered"
};

#endif // CONSTANTS_H
