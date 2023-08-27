/**
 * @file configureBT.cpp
 * 
 * HC-05/06 AT Command Center
 *  Version 3.0
 * 
 *  Description: Simple HC05/06 AT configuration program. Requires 2nd UART (Serial1) defined. 
 * 
 * 
 *  Created on: 18-Oct, 2021
 *      Author: miller4@rose-hulman.edu
 *    Modified: 25-Aug, 2023
 *    Revision: 3.0
 */
// uncomment following line to include debugging output to serial
//#define DEBUG           1

#include "configureBT.h"
#include "include/constants.h"
#include "include/hc05.h"

// global variables
int deviceModel = MODEL_UNKNOWN;
int firmVersion = FIRM_UNKNOWN;
int deviceRole = ROLE_UNKNOWN;
int baudRate = VERS2_MIN_BAUD;
int parity = NOPARITY;
int stopBits = STOP1BIT;
String hc06Version = "";
String nameBT, command;


void setup() {
  // configure Serial Monitor UART (57600 8N1)
  Serial.begin(57600);
  delay(1000);
  // configure Serial1 UART interface to HC-xx
  Serial1.begin(baudRateList[baudRate], parityList[parity]);
  delay(CONFIG_DELAY); 
  Serial.flush();
  disableCMDpin();
  
}

void commandMenu() {
  // clear any existing messages in buffer
  clearInputStream(firmVersion);

  printMenu();
  // check for user selection of menu option
  while (Serial.available() == 0);
  command = Serial.readString();
  switch (command.toInt()) {
    case 1:
      setBaudRate();
      break;
    case 2:
      setName();
      break;
    case 3:
      setPin();
      break;
    case 4:
      setParity();
      break;
    case 5:
      setLocalBaud();
      break;
    case 6:
      setLocalParity();
      break;
    case 7:
      getVersion();
      break;
    case 8:
      scanDevice();
      break;
    default:
      Serial.println("Invalid entry");
      break;
  }
  Serial.println();
  
  delay(SHORT_DELAY);
}

/**
 * setCommandMode
 *
 * @brief Set CMD pin high to place HC-05 in command mode.
 */
void setCommandMode() {
  pinMode(CMD_PIN, OUTPUT);
  digitalWrite(CMD_PIN, MODE_COMMAND);
}

/**
 * setDataMode
 *
 * @brief Set CMD pin low to place HC-05 in data mode.
 */
void setDataMode() {
  // TODO use disableCMDpin instead?
  pinMode(CMD_PIN, OUTPUT);
  digitalWrite(CMD_PIN, MODE_DATA);
}

/**
 * disableCMDpin
 *
 * @brief Disconnect output to CMD pin.
 */
void disableCMDpin() {
  pinMode(CMD_PIN, INPUT);  // set to input allow CMD pin to float
}

/**
 * responseDelay
 *  
 * @brief Delays for period necessary to allow completion of HC-xx response to command.
 * 
 * @param characters  count of characters in AT command
 * @param firmware    firmware version identifier for HC-xx
 * @param command     index of AT command (as defined in HCxxCommands)
 */
void responseDelay(unsigned long characters, int firmware, HCxxCommands command) {
  if ((baudRate < 0) || (baudRate >= BAUD_LIST_CNT)) return;
  unsigned long writeMS = (characters + responseChars[command]) * BITS_PER_CHAR * 1000 
                              / baudRateList[baudRate];
  delay(writeMS + responseMS[firmware]);
}

/**
 * clearInputStream
 *  
 * @brief Clears Serial1 input buffers before requesting new response.
 * 
 * @param firmware    firmware version identifier for HC-xx
 */
void clearInputStream(int firmware) {
  if (firmware == FIRM_VERSION2) {
    // ensure HC06 is not waiting for termination of partially complete command
    Serial1.print(lineEnding[FIRM_VERSION2]);
    Serial1.flush();
    delay(FW2_RESPONSE);
  }
  while (Serial1.available() > 0) {
    // wait until input stream is clear
    Serial1.read();
  }
}

/**
 * printMenu
 *  
 * Print menu of options for configuration of UART or Bluetooth module. 
 * Will first check for identified connected device, and request scan if
 * firmware and configuration of connected device is not known.
 */
void printMenu() {
  while (VERSION_UNKNOWN) {
    if (scanDevice()) break;
    Serial.println();
    Serial.println("Device version/configuration unknown.");
    Serial.println("Check connections and hit any key to scan again.");
  }
  Serial.println();
  Serial.println();
  Serial.write(12);   // Form feed (not supported in Serial Monitor)
  Serial.print("Device found - Version: ");
  Serial.println(hc06Version);
  Serial.print("Current baud rate: ");
  Serial.println(baudRateList[baudRate]);
  Serial.print("Current parity: ");
  Serial.println(parityType[parity]);
  Serial.println();
  Serial.println("Select option:");
  for (int i = 1; i < HC06_MENUSIZE, i++) {
    Serial.println("\t(" + i + hc06Menu[i]);
  }
  Serial.println();
}

bool scanDevice() {
  String comBuffer;
  firmVersion = FIRM_UNKNOWN;
  hc06Version = "";
  
  Serial1.end();
  delay(CONFIG_DELAY);
  Serial.print("\nSearching for firmware and version of HC06");
  // Scan through possible UART configurations for each firmware version. 
  //  Use AT command to test for OK response.
  for (int firmware = FIRM_VERSION2; firmware > FIRM_UNKNOWN; firmware--) {
    for (parity = NOPARITY; parity < PARITY_LIST_CNT; parity++) {
      // firmware version 2.x/3.x does not support baud rate below 4800
      if (firmware == FIRM_VERSION2) {
        baudRate = VERS2_MIN_BAUD;
      } else {
        baudRate = 0;
      }
      for ( ; baudRate < BAUD_LIST_CNT; baudRate++) {
        // Test for Version x.x firmware AT echo
        command = atCommands[ECHO] + lineEnding[firmware]; // AT command
#ifdef DEBUG
        // debugging instructions to verify characters sent to UART
        Serial.print("\nCommand length: ");
        Serial.println(command.length());
        for (unsigned int i = 0; i < command.length(); i++) {
          Serial.print("\t");
          Serial.print(command.charAt(i), HEX);
        }
        Serial.println();
#endif
        Serial.print(" .");
        // set to new baud rate and parity setting and test connection
        Serial1.begin(baudRateList[baudRate], parityList[parity]);
        delay(CONFIG_DELAY);
        clearInputStream(firmware);
        Serial1.print(command);
        Serial1.flush();
        responseDelay(command.length(), firmware, ECHO);
        if (Serial1.available() > 0) {
          comBuffer = Serial1.readString();
#ifdef DEBUG
          Serial.println();
          Serial.println(comBuffer);
          for (unsigned int i = 0; i < comBuffer.length(); i++) {
            Serial.print("\t");
            Serial.print(comBuffer.charAt(i), HEX);
          }
          Serial.println();
#endif
          // if OK response received, UART configuration found
          if (comBuffer.startsWith(STATUS_OK)) {
            firmVersion = firmware;
            // firmware version 2.x/3.x might be hc-05 device
            if (firmware == FIRM_VERSION2) {
              // TODO call getRole then setRole
              switch (getRole()) {
                case ROLE_SLAVE:
                    // hc-05 fw vers 2/3 will fail when attempting to set role
                    if (setRole(ROLE_SLAVE)) {
                      deviceModel = MODEL_HC05;
                    } else {
                      deviceModel = MODEL_HC06;
                    }
                    break;
                case ROLE_MASTER:
                case ROLE_SLAVE_LOOP:
                    deviceModel = MODEL_HC05;
                    break;
                default:
                    deviceModel = MODEL_HC06;
                    break;
              }
            }
            break;
          }
        }
        while (Serial1.available() > 0) {
          // wait until input stream is clear
          Serial1.read();
        }
        // end Test for Version x.x firmware
        Serial1.end();
        delay(CONFIG_DELAY);
      } // end baud rate loop
      if (VERSION_KNOWN)  break;
    } // end parity loop
    if (VERSION_KNOWN)  break;
  } // end firmware loop
  Serial.println();

  // If configuration successfully determined, update firmware version string
  if (VERSION_KNOWN) {
    command = atCommands[HCVERSION] + requestVal[firmVersion];
    Serial1.print(command);
    Serial1.flush();
    responseDelay(command.length(), firmVersion, HCVERSION);
    if (Serial1.available() > 0) {
      hc06Version = Serial1.readString();
    }
  }
  
  return (VERSION_KNOWN);
}

void testEcho() {
  String comBuffer = "";
  command = atCommands[ECHO] + lineEnding[firmVersion];
  clearInputStream(firmVersion);
  Serial1.print(command);
  Serial1.flush();
  responseDelay(command.length(), firmVersion, ECHO);
  if (Serial1.available() > 0) {
    comBuffer = Serial1.readString();
    Serial.print("[HC06]: ");
    Serial.println(comBuffer);
#ifdef DEBUG
    for (unsigned int i = 0; i < comBuffer.length(); i++)
    {
      Serial.print("\t");
      Serial.print(comBuffer.charAt(i), HEX);
    }
    Serial.println();
#endif
  }
  if (!(comBuffer.startsWith(STATUS_OK))) {
    Serial.println("OK response not received.");
    firmVersion = FIRM_UNKNOWN;
  }
  Serial.println("\nEnter any character to return to menu.");
  while (Serial.available() < 1);
  delay(SHORT_DELAY);
  Serial.readString();   // clear buffer
}

int getRole() {
  String comBuffer = "";
  int role;
  command = String(ROLE_REQ);
  clearInputStream(firmVersion);
  Serial1.print(command);
  Serial1.flush();
  responseDelay(command.length(), firmVersion, ECHO);
  if (Serial1.available() > 0) {
    comBuffer = Serial1.readString();
    Serial.println("\nRequesting device role.");
    Serial.print("[HC06]: ");
    Serial.println(comBuffer);
#ifdef DEBUG
    for (unsigned int i = 0; i < comBuffer.length(); i++)
    {
      Serial.print("\t");
      Serial.print(comBuffer.charAt(i), HEX);
    }
    Serial.println();
#endif
  }
  role = comBuffer.indexOf(':');
  if (role < 0) {
    deviceRole = ROLE_UNKNOWN;
  } else {
    switch (comBuffer.charAt(role)) {
      case '0': deviceRole = ROLE_SLAVE;
                break;
      case '1': deviceRole = ROLE_MASTER;
                break;
      case '2': deviceRole = ROLE_SLAVE_LOOP;
                break;
      default:  deviceRole = ROLE_UNKNOWN;
    }
  }
  if (deviceRole == ROLE_UNKNOWN) {
    Serial.println("Role response not identified.");
  } else {
    Serial.println("Device role is: " + roleString[deviceRole]);
  }
  return deviceRole;
}

bool setRole(unsigned int role) {
  String comBuffer = "";
  if (role > ROLE_SLAVE_LOOP) 
    return false;
  command = String(ROLE_CMD) + role + lineEnding[FIRM_VERSION2];
//  Serial.print("Set role of HC05 to ");
//  Serial.println(roleString[role]);
  clearInputStream(firmVersion);
  Serial1.print(command);
  Serial1.flush();
  responseDelay(command.length(), firmVersion, ECHO);
  if (Serial1.available() > 0) {
    comBuffer = Serial1.readString();
    Serial.print("[HC06]: ");
    Serial.println(comBuffer);
#ifdef DEBUG
    for (unsigned int i = 0; i < comBuffer.length(); i++)
    {
      Serial.print("\t");
      Serial.print(comBuffer.charAt(i), HEX);
    }
    Serial.println();
#endif
  }
  if (!(comBuffer.startsWith(STATUS_OK))) {
    Serial.println("Device role not set.");
//    deviceRole = ROLE_UNKNOWN;  // don't modify role since it may be HC06
    return false;
  }
  deviceRole = role;
  Serial.println("Device role set to: " + roleString[deviceRole]);
  return true;
}

/**
 * setLocalBaud
 *  
 * Manually configure baud rate of Serial1, for testing/debugging purposes. 
 * Preferred to allow scanDevice() to automatically set configuration.
 */
void setLocalBaud() {
  String comBuffer = "";
  Serial.println("It is advised that baud rate is left at same setting as found hardware.");
  Serial.print("Current baud rate: ");
  Serial.println(baudRateList[baudRate]);
  Serial.println("Select desired baud rate:");
  Serial.println("\t(0) Cancel");
  Serial.println("\t(1)---------1200");
  Serial.println("\t(2)---------2400");
  Serial.println("\t(3)---------4800");
  Serial.println("\t(4)---------9600 (Default)");
  Serial.println("\t(5)---------19200");
  Serial.println("\t(6)---------38400");
  Serial.println("\t(7)---------57600");
  Serial.println("\t(8)---------115200");
  Serial.println();

  while (Serial.available() < 1);
  command = Serial.readString();
  int tempBaud = command.toInt() - 1;
  if (tempBaud < 0) {
    Serial.println("Canceled");
  } else if (tempBaud < BAUD_LIST_CNT) {
    Serial1.end();
    delay(CONFIG_DELAY);
    baudRate = tempBaud;
    Serial1.begin(baudRateList[baudRate], parityList[parity]);
    delay(CONFIG_DELAY);
    Serial.print("Set local baud rate to ");
    Serial.println(baudRateList[baudRate]);
    Serial.println("Testing new parity configuration . . .");
    testEcho();
  } else {
    Serial.println("Invalid entry");
  }
  
}

/**
 * setLocalParity
 *  
 * Manually configure parity of Serial1, for testing/debugging purposes. 
 * Preferred to allow scanDevice() to automatically set configuration.
 */
void setLocalParity() {
  String comBuffer = "";
  Serial.println("It is advised that parity is left at same setting as found hardware.");
  Serial.print("Current parity: ");
  Serial.println(parityType[parity]);
  Serial.println("Select parity option:");
  Serial.println("\t(0) Cancel");
  Serial.println("\t(1).......No parity");
  Serial.println("\t(2).......Odd parity");
  Serial.println("\t(3).......Even parity");
  Serial.println();

  while (Serial.available() < 1);
  command = Serial.readString();
  int tempParity = command.toInt() - 1;
  if (tempParity < 0) {
    Serial.println("Canceled");
  } else if (tempParity < PARITY_LIST_CNT) {
    Serial1.end();
    delay(CONFIG_DELAY);
    parity = tempParity;
    Serial.println("Setting to " + parityType[parity] + " Parity check");
    Serial1.begin(baudRateList[baudRate], parityList[parity]);
    delay(CONFIG_DELAY);
    Serial.println("Testing new parity configuration . . .");
    testEcho();
  } else {
    Serial.println("Invalid entry");
  }
  
}

/**
 * getVersion
 *  
 * Send AT command to request firmware version to Serial1 and display response.
 */
void getVersion() {
  String comBuffer = "";
  command = atCommands[HCVERSION] + requestVal[firmVersion];
  clearInputStream(firmVersion);
  Serial1.print(command);
  Serial1.flush();
  responseDelay(command.length(), firmVersion, HCVERSION);
  if (Serial1.available() > 0) {
    comBuffer = Serial1.readString();
    Serial.print("\n[HC06]: ");
    Serial.println(comBuffer);
    Serial.println();
#ifdef DEBUG
    for (unsigned int i = 0; i < comBuffer.length(); i++)
    {
      Serial.print("\t");
      Serial.print(comBuffer.charAt(i), HEX);
    }
    Serial.println();
#endif
  }
  testEcho();
}

/**
 * constructUARTstring
 *  
 * @brief Constructs string for AT command to configure UART (for firmware vers 3.x)
 * 
 * @param baud    baud rate value (e.g. 57600)
 * @param prty    parity setting
 *                - 0 - None
 *                - 1 - Odd parity
 *                - 2 - Even parity
 * @param stops   number of stop bits
 *                - 0 - 1 bit
 *                - 1 - 2 bits
 * 
 *  @returns String for AT command
 */
String constructUARTstring(int baud, int prty, int stops) {
  return String(UART_CMD) + baudRateList[baud] + "," + stops + "," 
            + (prty) + lineEnding[FIRM_VERSION2];
}

/**
 * setBaudRate
 *  
 * Configure baud rate of HC-xx UART.
 * Sends AT command to configure baud rate of HC-xx UART and displays response.
 * If successful, updates Serial1 configuration to new UART settings.
 */
void setBaudRate() {
  String comBuffer = "";
  Serial.print("Current baud rate: ");
  Serial.println(baudRateList[baudRate]);
  Serial.println("Select desired baud rate:");
  Serial.println("\t(0) Cancel");
  Serial.println("\t(1)---------1200");
  Serial.println("\t(2)---------2400");
  Serial.println("\t(3)---------4800");
  Serial.println("\t(4)---------9600 (Default)");
  Serial.println("\t(5)---------19200");
  Serial.println("\t(6)---------38400");
  Serial.println("\t(7)---------57600");
  Serial.println("\t(8)---------115200");
  Serial.println();

  while (Serial.available() < 1);
  command = Serial.readString();
  int tempBaud = command.toInt() - 1;
  if (tempBaud < 0) {
    Serial.println("Canceled");
  } else if (tempBaud < BAUD_LIST_CNT) {
    // construct AT command for UART configuration based on firmware version
    if (firmVersion == FIRM_VERSION2) {
      // firmware version 3.x does not support baud rate below 4800
      if (tempBaud < VERS2_MIN_BAUD) {
        Serial.println("\nBaud rates below 4800 not supported by this firmware.");
        delay(MENU_DELAY);
        return;
      }
      command = constructUARTstring(tempBaud, parity, stopBits);
    } else {
      command = String(BAUD_CMD) + (tempBaud+1) + lineEnding[firmVersion];
    }
    Serial.print("Setting HC06 and local baud rate to ");
    Serial.println(baudRateList[tempBaud]);
    Serial.println("\tsending command: " + command);
    Serial.println();
    clearInputStream(firmVersion);
    Serial1.print(command);
    Serial1.flush();
    responseDelay(command.length(), firmVersion, BAUD_SET);
    if (Serial1.available() > 0) {
      comBuffer = Serial1.readString();
      Serial.print("[HC06]: ");
      Serial.println(comBuffer);
      Serial.println();
#ifdef DEBUG
      for (unsigned int i = 0; i < comBuffer.length(); i++) {
        Serial.print("\t");
        Serial.print(comBuffer.charAt(i), HEX);
      }
      Serial.println();
#endif
    }
    if (!(comBuffer.startsWith(STATUS_OK))) {
      Serial.println("\nRequest failed.");
      delay(MENU_DELAY);
      return;
    }
    // if OK response received, change Serial1 UART settings to match HC-xx
    Serial1.end();
    baudRate = tempBaud;
    delay(CONFIG_DELAY);
    Serial1.begin(baudRateList[baudRate], parityList[parity]);
    delay(CONFIG_DELAY);
    Serial.println("Testing new baud rate configuration . . .");
    testEcho();
  } else {
    Serial.println("Invalid entry");
  }
  
}

/**
 * setName
 *  
 * Configure name of Bluetooth module.
 * Sends AT command to set Bluetooth broadcast name of HC-xx device. Prepends 
 * 'HC06_' to user input string. Some devices with firmware version 1.x 
 * exhibited failures when trying to set name to more than 14 characters with
 * higher baud rates.
 */
void setName() {
  String comBuffer = "";
  Serial.println("Enter BT name (max 15 characters - prepends HC06_): ");

  while (Serial.available() < 1);
  nameBT = Serial.readString();
  nameBT.trim();  // remove leading or trailing whitespaces (newline characters)
  if (nameBT.length() > 0) {
    // prepend user provided string with HC06_ to produce max 20 character name
    nameBT = "HC06_" + nameBT.substring(0, 15);
    Serial.print("Setting name to ");
    Serial.println(nameBT);
    command = atCommands[BTNAME] + setValue[firmVersion] + nameBT + lineEnding[firmVersion];
    Serial.println("\tsending command: " + command);
    clearInputStream(firmVersion);
    Serial1.print(command);
    Serial1.flush();
    responseDelay(command.length(), firmVersion, BTNAME);
    while (Serial1.available() > 0) {
      comBuffer = Serial1.readString();
      Serial.print("[HC06]: ");
      Serial.println(comBuffer);
      Serial.println();
#ifdef DEBUG
      for (unsigned int i = 0; i < comBuffer.length(); i++) {
        Serial.print("\t");
        Serial.print(comBuffer.charAt(i), HEX);
      }
      Serial.println();
#endif
    }
    if (!(comBuffer.startsWith(STATUS_OK))) {
      Serial.println("Names above 14 characters fail for some FW Version 1.x baud settings.");
      Serial.println("Try with alternate string less than 10 characters.");
    }
  } else {
    Serial.println("Invalid entry (empty string)");
  }
  
  Serial.println("\nEnter any character to return to menu.");
  while (Serial.available() < 1);
  delay(SHORT_DELAY);
  Serial.readString();   // clear buffer
}

/**
 * setPin
 *  
 * Configure Bluetooth pin of HC-xx device.
 * Sends AT command to configure BT pin/passkey of HC-xx UART device.
 * For firmware version 1.x, 4-digit code is accepted. For firmware version
 * 3.x, up to 16 alphanumeric character passkey is accepted according to 
 * documentation. This is artificially limited to 14 characters to ensure no
 * conflict with adding quotation characters.
 */
void setPin() {
  String pin;
  if (firmVersion == FIRM_VERSION2) {
    Serial.println("Enter new BT passkey (14 characters max): ");
  } else {
    Serial.println("Enter new pin number (4 digits): ");
  }

  while (Serial.available() < 1);
  pin = Serial.readString();
  pin.trim();
  if (firmVersion == FIRM_VERSION2) {
    // version 3.x FW appears to require quotes around passkey,
    //  though this isn't indicated in documentation
    //  https://forum.arduino.cc/t/password-hc-05/481294
    pin = String("\"") + pin.substring(0, 14) + String("\"");
  } else if (pin.length() == 4) {
    // for firware version 1.x, verify 4 numeric characters received
    for (unsigned int i = 0; i < 4; i++) {
      if (!isDigit(pin.charAt(i))) {
        Serial.println("\nInvalid entry (not 4-digit integer)");
#ifdef DEBUG
        Serial.print("\tCharacters: ");
        for (i = 0; i < pin.length(); i++) {
          Serial.print(pin[i], HEX);
          Serial.print(" ");
        }
        Serial.println();
#endif
        delay(MENU_DELAY);
        return;
      }
    }
  } else {
    Serial.println("\nInvalid entry (not 4-digit integer)");
#ifdef DEBUG
    Serial.print("\tCharacters: ");
    for (unsigned int i = 0; i < pin.length(); i++) {
      Serial.print(pin[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
#endif
    delay(MENU_DELAY);
    return;
  }

  Serial.print("Setting pin to ");
  Serial.println(pin);
  if (firmVersion == FIRM_VERSION1) {
    command = atCommands[BTPIN] + pin;
  } else {
    command = atCommands[BTPSWD] + setValue[FIRM_VERSION2] + pin + lineEnding[FIRM_VERSION2];
  }
  
  Serial.println("\tsending command: " + command);
  clearInputStream(firmVersion);
  Serial1.print(command);
  Serial1.flush();
  responseDelay(command.length(), firmVersion, BTPIN);
  while (Serial1.available() > 0) {
    Serial.print("[HC06]: ");
    Serial.println(Serial1.readString());
    Serial.println();
  }
  Serial.println("\nEnter any character to return to menu.");
  while (Serial.available() < 1);
  delay(SHORT_DELAY);
  Serial.readString();   // clear buffer
}

/**
 * setParity
 *  
 * Configure parity of HC-xx UART.
 * Sends AT command to configure parity of HC-xx UART and displays response.
 * If successful, updates Serial1 configuration to new UART settings.
 */
void setParity() {
  String comBuffer = "";
  Serial.print("Current parity: ");
  Serial.println(parityType[parity]);
  Serial.println("Select parity option:");
  Serial.println("\t(0) Cancel");
  Serial.println("\t(1).......No parity");
  Serial.println("\t(2).......Odd parity");
  Serial.println("\t(3).......Even parity");
  Serial.println();

  while (Serial.available() < 1);
  command = Serial.readString();
  int tempParity = command.toInt() - 1;
  if (tempParity < 0) {
    Serial.println("Canceled");
  } else if (tempParity < PARITY_LIST_CNT) {
    // construct AT command for UART configuration based on firmware version
    if (firmVersion == FIRM_VERSION2) {
      command = constructUARTstring(baudRate, tempParity, stopBits);
    } else {
      command = parityCmd[tempParity];
    }
    Serial.println("Setting to " + parityType[tempParity] + " Parity check");
    Serial.println("\tsending command: " + command);
    clearInputStream(firmVersion);
    Serial1.print(command);
    Serial1.flush();
    responseDelay(command.length(), firmVersion, PARITY_SET);

    if (Serial1.available() > 0) {
      comBuffer = Serial1.readString();
      Serial.print("[HC06]: ");
      Serial.println(comBuffer);
      Serial.println();
#ifdef DEBUG
      for (unsigned int i = 0; i < comBuffer.length(); i++) {
        Serial.print("\t");
        Serial.print(comBuffer.charAt(i), HEX);
      }
      Serial.println();
#endif
    }
    if (!(comBuffer.startsWith(STATUS_OK))) {
      Serial.println("\nRequest failed.");
      delay(MENU_DELAY);
      return;
    }
    // if OK response received, change Serial1 UART settings to match HC-xx
    Serial1.end();
    parity = tempParity;
    delay(CONFIG_DELAY);
    // firmware version 1.x requires power-cycle of HC-06 to update parity settings
    if (firmVersion == FIRM_VERSION1) {
      Serial.println("To complete change of parity, remove then reconnect power to HC-06.");
      Serial.println("Enter any character when complete (LED should be blinking).");
      while (Serial.available() < 1);
      Serial.readString();   // clear buffer
    }
    Serial1.begin(baudRateList[baudRate], parityList[parity]);
    delay(CONFIG_DELAY);
    Serial.println("Testing new parity configuration . . .");
    testEcho();
  } else {
    Serial.println("Invalid entry");
  }
  
}
