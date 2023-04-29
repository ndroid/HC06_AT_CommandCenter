/*
 * HC-05/06 AT Command Center
 * 
 *  Description: Header file for HC-05 related constants. 
 * 
 *  Created on: 28-Apr, 2023
 *      Author: miller4@rose-hulman.edu
 *    Modified: 28-Apr, 2023
 */

#ifndef HC05_H
#define HC05_H

// HC-05 modes of operation:
//  Data mode -     CMD pin pulled to logic low level or unconnected
//  Command mode -  CMD pin pulled to logic high level
#define CMD_PIN         8           // Arduino pin connected to CMD pin of HC-05
#define MODE_DATA       LOW         // HC-05 in data mode
#define MODE_COMMAND    HIGH        // HC-05 in command mode

const char errorCodes[] = {
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

// global variables
//int cmdPin = CMD_PIN;

#endif // HC05_H
