/**
 * @file configureBT.cpp
 * 
 * HC-05/06 AT Command Center
 *  Version 2.0
 * 
 *  Description: Simple HC05/06 AT configuration program. Requires 2nd UART (Serial1) defined. 
 * 
 * 
 *  Created on: 18-Oct, 2021
 *      Author: miller4@rose-hulman.edu
 *    Modified: 9-Sep, 2023
 *    Revision: 2.0
 */
// uncomment following line to include debugging output to serial
//#define DEBUG           1

#include "configureBT.h"
#include "include/constants.h"


HCBT::HCBT(Stream uart, int statePin, int keyPin) {
  _uart = uart;
  _statePin = statePin;
  _keyPin = keyPin;
  initDevice();
  if (statePin > 0) pinMode(statePin, INPUT);
  if (keyPin > 0) pinMode(keyPin, OUTPUT);
}

void HCBT::initDevice() {
  deviceModel = MODEL_UNKNOWN;
  firmVersion = FIRM_UNKNOWN;
  deviceRole = ROLE_UNKNOWN;
  baudRate = VERS2_MIN_BAUD;
  parity = NOPARITY;
  stopBits = STOP1BIT;
  versionString = "";
  btName = "";
}

void HCBT::commandMenu() {
  String command;
  // clear any existing messages in buffer
  clearStreams();

  printMenu();
  // check for user selection of menu option
  while (Serial.available() == 0);
  command = Serial.readString();
  switch (command.toInt()) {
    case 1:
      selectBaudRate();
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
      detectDevice();
      break;
    default:
      Serial.println("Invalid entry");
      break;
  }
  Serial.println();
  
  delay(SHORT_DELAY);
}

void HCBT::setCommandMode() {
  if (_keyPin > 0) {
    pinMode(_keyPin, OUTPUT);
    digitalWrite(_keyPin, MODE_COMMAND);
  }
}

void HCBT::setDataMode() {
  if (_keyPin > 0) {
    digitalWrite(_keyPin, MODE_DATA);
    // low or floating signal disables command mode
    pinMode(_keyPin, INPUT);
  }
}

void HCBT::responseDelay(unsigned long characters, int firmware, HCxxCommands command) {
  if ((baudRate < 0) || (baudRate >= BAUD_LIST_CNT)) return;
  unsigned long writeMS = (characters + responseChars[command]) * BITS_PER_CHAR * 1000 
                              / baudRateList[baudRate];
  delay(writeMS + responseMS[firmware]);
}

void HCBT::clearStreams() {
  while (Serial.available() > 0) {
    // wait until input stream is clear
    Serial.read();
  }
  clearInputStream(firmVersion);
}

void HCBT::clearInputStream(int firmware) {
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

void HCBT::printMenu() {
  while (VERSION_UNKNOWN) {
    if (detectDevice())   break;
    Serial.println();
    Serial.println("Device version/configuration unknown.");
    Serial.println("Check connections and enter any character to scan again.");
    while (Serial.available() < 1);
    delay(SHORT_DELAY);
    Serial.readString();   // clear buffer
  }
  Serial.println("\n");
  Serial.write(12);   // Form feed (not supported in Serial Monitor)
  Serial.println(responsePrefix[deviceModel] + versionString);
  Serial.print("\tBaud rate: ");
  Serial.println(baudRateList[baudRate]);
  Serial.print("\tParity: ");
  Serial.println(parityType[parity]);
  Serial.println();
  Serial.println("Select option:");
  for (int i = 1; i < HC06_MENUSIZE, i++) {
    Serial.println("\t(" + i + hc06Menu[i]);
  }
  Serial.println();
}

bool HCBT::detectDevice(bool verboseOut) {
  String command;
  String comBuffer;

  initDevice();
  Serial1.end();
  delay(CONFIG_DELAY);
  if (verboseOut) {
    Serial.print("\nSearching for firmware and version of HC0x device");
  }
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
        if (verboseOut) {
          Serial.print(" .");
        }
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
              // use getRole and setRole response to identify device model
              switch (fetchRole(verboseOut)) {
                case ROLE_SLAVE:
                    // hc-05 fw vers 2/3 will fail when attempting to set role
                    if (setRole(ROLE_SLAVE, verboseOut)) {
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
  if (verboseOut) {
    Serial.println();
  }
  // If configuration successfully determined, update firmware version string
  fetchVersion(verboseOut);
  if (verboseOut) {
    if (VERSION_KNOWN) {
      Serial.println("\nDevice identified . . .");
      Serial.print("\tModel: "); 
      Serial.println(responsePrefix[deviceModel] + versionString);
      Serial.print("\tBaud rate: "); 
      Serial.println(baudRateList[baudRate]);
      Serial.print("\tParity: "); 
      Serial.println(parityType[parity]);
    } else {
      Serial.println("\nDevice not identified. Check connections and try again.");
    }
  }
  
  return (VERSION_KNOWN);
}

bool HCBT::testEcho(bool verboseOut) {
  String comBuffer = "";
  String command;

  command = atCommands[ECHO] + lineEnding[firmVersion];
  clearInputStream(firmVersion);
  Serial1.print(command);
  Serial1.flush();
  responseDelay(command.length(), firmVersion, ECHO);
  if (Serial1.available() > 0) {
    comBuffer = Serial1.readString();
    if (verboseOut) {
      Serial.print(responsePrefix[deviceModel]);
      Serial.println(comBuffer);
    }
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
    if (verboseOut) {
      Serial.println("OK response not received.");
    }
    initDevice();
  }
}

int HCBT::fetchRole(bool verboseOut) {
  String comBuffer = "";
  String command;
  int role;

  if (VERSION_UNKNOWN)  
    return ROLE_UNKNOWN;
  command = String(ROLE_REQ);
  clearInputStream(firmVersion);
  Serial1.print(command);
  Serial1.flush();
  // response is OK, same as AT
  responseDelay(command.length(), firmVersion, ECHO);
  if (Serial1.available() > 0) {
    comBuffer = Serial1.readString();
    if (verboseOut) {
      Serial.println("\nRequesting device role.");
      Serial.print(responsePrefix[deviceModel]);
      Serial.println(comBuffer);
    }
#ifdef DEBUG
    for (unsigned int i = 0; i < comBuffer.length(); i++) {
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
    switch (comBuffer.charAt(role+1)) {
      case '0': deviceRole = ROLE_SLAVE;
                break;
      case '1': deviceRole = ROLE_MASTER;
                break;
      case '2': deviceRole = ROLE_SLAVE_LOOP;
                break;
      default:  deviceRole = ROLE_UNKNOWN;
    }
  }
  if (verboseOut) {
    if (deviceRole == ROLE_UNKNOWN) {
      Serial.println("Role response not identified.");
    } else {
      Serial.println("Device role is: " + roleString[deviceRole]);
    }
  }
  return deviceRole;
}

int HCBT::getRole(bool verboseOut) {
  if (deviceRole == ROLE_UNKNOWN) {
    return fetchRole(verboseOut);
  }
  return deviceRole;
}

bool HCBT::setRole(unsigned int role, bool verboseOut) {
  String comBuffer = "";
  String command;

  if (VERSION_UNKNOWN)  
    return false;
  if ((role < ROLE_SLAVE) || (role > ROLE_SLAVE_LOOP))
    return false;
  if (deviceModel != MODEL_HC05) {
    if (role == ROLE_SLAVE)
      return true;
    else 
      return false;
  }
  command = String(ROLE_CMD) + role + lineEnding[FIRM_VERSION2];
  if (verboseOut) {
    Serial.print("Set role of HC05 to ");
    Serial.println(roleString[role]);
  }
  clearInputStream(firmVersion);
  Serial1.print(command);
  Serial1.flush();
  // response is OK, same as AT
  responseDelay(command.length(), firmVersion, ECHO);
  if (Serial1.available() > 0) {
    comBuffer = Serial1.readString();
    if (verboseOut) {
      Serial.print(responsePrefix[deviceModel]);
      Serial.println(comBuffer);
    }
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
    if (verboseOut) {
      Serial.println("Device role not set.");
    }
    //deviceRole = ROLE_UNKNOWN;  // don't modify role since it may be HC06
    return false;
  }
  deviceRole = role;
  if (verboseOut) {
    Serial.println("Device role set to: " + roleString[deviceRole]);
  }
  return true;
}

void HCBT::setLocalBaud() {
  String comBuffer = "";
  String command;

  clearStreams();
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
//    Serial.println("Testing new baud configuration . . .");
//    testEcho(true);
  } else {
    Serial.println("Invalid entry");
  }
}

void HCBT::setLocalParity() {
  String comBuffer = "";
  String command;

  clearStreams();
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
//    Serial.println("Testing new parity configuration . . .");
//    testEcho();
  } else {
    Serial.println("Invalid entry");
  }
}

String HCBT::getVersion(bool verboseOut) {
  if (versionString.equals(""))
    return fetchVersion(verboseOut);
  return versionString;
}

String HCBT::fetchVersion(bool verboseOut) {
  String comBuffer = "";
  String command;

  if (VERSION_UNKNOWN) 
    return "";
  command = atCommands[HCVERSION] + requestVal[firmVersion];
  clearInputStream(firmVersion);
  Serial1.print(command);
  Serial1.flush();
  responseDelay(command.length(), firmVersion, HCVERSION);
  if (Serial1.available() > 0) {
    comBuffer = Serial1.readString();
    if (verboseOut) {
      Serial.print(responsePrefix[deviceModel]);
      Serial.println(comBuffer);
      Serial.println();
    }
    versionString = comBuffer;
#ifdef DEBUG
    for (unsigned int i = 0; i < comBuffer.length(); i++)
    {
      Serial.print("\t");
      Serial.print(comBuffer.charAt(i), HEX);
    }
    Serial.println();
#endif
  } else {
    testEcho(verboseOut);
  }
}

String HCBT::constructUARTstring(int baud, int prty, int stops) {
  return String(UART_CMD) + baudRateList[baud] + "," + stops + "," 
            + (prty) + lineEnding[FIRM_VERSION2];
}

void HCBT::selectBaudRate() {
  String command;

  clearStreams();
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
  int tempBaud = command.toInt();
  if (tempBaud <= 0) {
    Serial.println("Canceled");
  } else if (tempBaud <= BAUD_LIST_CNT) {
    setBaudRate(tempBaud, true);
  } else {
    Serial.println("Invalid entry");
  }
}

bool HCBT::setBaudRate(int newBaud, bool verboseOut) {
  String comBuffer = "";
  String command;

  if (VERSION_UNKNOWN) 
    return false;
  if (newBaud > BAUD_LIST_CNT) {
    if (verboseOut) {
      Serial.println("\nBaud rates above 115200 not supported.");
      Serial.println("See docmentation for valid index values.");
      delay(MENU_DELAY);
    }
    return false;
  }
  newBaud -= 1;
  // construct AT command for UART configuration based on firmware version
  if (firmVersion == FIRM_VERSION2) {
    // firmware version 3.x does not support baud rate below 4800
    if (newBaud < VERS2_MIN_BAUD) {
      if (verboseOut) {
        Serial.println("\nBaud rates below 4800 not supported by this firmware.");
        delay(MENU_DELAY);
      }
      return false;
    }
    command = constructUARTstring(newBaud, parity, stopBits);
  } else {
    command = String(BAUD_CMD) + (newBaud+1) + lineEnding[firmVersion];
  }
  if (verboseOut) {
    Serial.print("Setting HC06 and local baud rate to ");
    Serial.println(baudRateList[newBaud]);
    Serial.println("\tsending command: " + command);
    Serial.println();
  }
  clearInputStream(firmVersion);
  Serial1.print(command);
  Serial1.flush();
  responseDelay(command.length(), firmVersion, BAUD_SET);
  if (Serial1.available() > 0) {
    comBuffer = Serial1.readString();
    if (verboseOut) {
      Serial.print(responsePrefix[deviceModel]);
      Serial.println(comBuffer);
      Serial.println();
    }
#ifdef DEBUG
    for (unsigned int i = 0; i < comBuffer.length(); i++) {
      Serial.print("\t");
      Serial.print(comBuffer.charAt(i), HEX);
    }
    Serial.println();
#endif
  }
  if (!(comBuffer.startsWith(STATUS_OK))) {
    if (verboseOut) {
      Serial.println("\nRequest failed.");
      delay(MENU_DELAY);
    }
    return false;
  }
  // if OK response received, change Serial1 UART settings to match HC-xx
  Serial1.end();
  baudRate = newBaud;
  delay(CONFIG_DELAY);
  Serial1.begin(baudRateList[baudRate], parityList[parity]);
  delay(CONFIG_DELAY);
  if (verboseOut) {
    Serial.println("Testing new baud rate configuration . . .");
  }
  return testEcho(verboseOut);
}

void HCBT::changeName() {
  String nameBT;
  int maxChars = 15;

  // Some devices with firmware version 1.x exhibited failures when trying to 
  //  set name to more than 14 characters at baud rates > 19200.
  if ((firmVersion == FIRM_VERSION1) && (baudRate > 4)) {
    maxChars = 9;
  }
  Serial.print("Enter BT name (max "); Serial.print(maxChars);
  Serial.println(" characters - prepends " + namePrefix[deviceModel] + "): ");

  while (Serial.available() < 1);
  nameBT = Serial.readString();
  nameBT.trim();  // remove leading or trailing whitespaces (newline characters)
  if (nameBT.length() > 0) {
    // prepend user provided string with HC0x_ to produce max 20 character name
    nameBT = namePrefix[deviceModel] + nameBT.substring(0, maxChars);
    setName(nameBT, true);
  } else {
    Serial.println("Invalid entry (empty string)");
  }
}

bool HCBT::setName(String newName, bool verboseOut) {
  String comBuffer = "";
  String command;

  if (VERSION_UNKNOWN) 
    return false;
  if (newName.length() > 0) {
    // Some devices with firmware version 1.x exhibited failures when trying to 
    //  set name to more than 14 characters at baud rates > 19200.
    if ((firmVersion == FIRM_VERSION1) && (baudRate > 4)) {
      btName = newName.substring(0, 14);
    } else {
      btName = newName.substring(0, 20);
    }
    if (verboseOut) {
      Serial.print("Setting name to ");
      Serial.println(btName);
    }
    command = atCommands[BTNAME] + setValue[firmVersion] + btName + lineEnding[firmVersion];
#ifdef DEBUG
    Serial.println("\tsending command: " + command);
#endif
    clearInputStream(firmVersion);
    Serial1.print(command);
    Serial1.flush();
    responseDelay(command.length(), firmVersion, BTNAME);
    if (Serial1.available() > 0) {
      comBuffer = Serial1.readString();
      if (verboseOut) {
        Serial.print(responsePrefix[deviceModel]);
        Serial.println(comBuffer);
        Serial.println();
      }
#ifdef DEBUG
      for (unsigned int i = 0; i < comBuffer.length(); i++) {
        Serial.print("\t");
        Serial.print(comBuffer.charAt(i), HEX);
      }
      Serial.println();
#endif
    }
    if (!(comBuffer.startsWith(STATUS_OK))) {
      if (verboseOut) {
        Serial.println("Names above 14 characters fail for some FW Version 1.x baud settings.");
        Serial.println("Try with alternate string less than 10 characters.");
      }
      return false;
    }
  } else {
    if (verboseOut) {
      Serial.println("Invalid entry (empty string)");
    }
    return false;
  }
  return true;
}

void HCBT::changePin() {
  String pin;

  if (firmVersion == FIRM_VERSION2) {
    Serial.println("Enter new BT passkey (14 characters max): ");
  } else {
    Serial.println("Enter new pin number (4 digits): ");
  }

  while (Serial.available() < 1);
  pin = Serial.readString();
  pin.trim();
  setPin(pin, true);
}

bool HCBT::setPin(String newPin, bool verboseOut) {
  String comBuffer = "";
  String command;

  if (VERSION_UNKNOWN) 
    return false;
  if (firmVersion == FIRM_VERSION2) {
    // TODO is there a min length for FW 3.x pin?
    if (newPin.length() < 1) {
      if (verboseOut) {
        Serial.println("\nInvalid entry (too few characters)");
        delay(MENU_DELAY);
      }
      return false;
    }
    // version 3.x FW appears to require quotes around passkey,
    //  though this isn't indicated in documentation
    //  https://forum.arduino.cc/t/password-hc-05/481294
    newPin = String("\"") + newPin.substring(0, 14) + String("\"");
  } else if (newPin.length() == 4) {
    // for firware version 1.x, verify 4 numeric characters received
    for (unsigned int i = 0; i < 4; i++) {
      if (!isDigit(newPin.charAt(i))) {
        if (verboseOut) {
          Serial.println("\nInvalid entry (not 4-digit integer)");
          delay(MENU_DELAY);
        }
#ifdef DEBUG
        Serial.print("\tCharacters: ");
        for (i = 0; i < newPin.length(); i++) {
          Serial.print(newPin[i], HEX);
          Serial.print(" ");
        }
        Serial.println();
#endif
        return false;
      }
    }
  } else {
    if (verboseOut) {
      Serial.println("\nInvalid entry (not 4-digit integer)");
      delay(MENU_DELAY);
    }
#ifdef DEBUG
    Serial.print("\tCharacters: ");
    for (unsigned int i = 0; i < newPin.length(); i++) {
      Serial.print(newPin[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
#endif
    return false;
  }

  if (verboseOut) {
    Serial.print("Setting pin to ");
    Serial.println(newPin);
  }
  if (firmVersion == FIRM_VERSION1) {
    command = atCommands[BTPIN] + newPin;
  } else {
    command = atCommands[BTPSWD] + setValue[FIRM_VERSION2] + newPin + lineEnding[FIRM_VERSION2];
  }
  
#ifdef DEBUG
  Serial.println("\tsending command: " + command);
#endif
  clearInputStream(firmVersion);
  Serial1.print(command);
  Serial1.flush();
  responseDelay(command.length(), firmVersion, BTPIN);
  if (Serial1.available() > 0) {
    comBuffer = Serial1.readString();
    if (verboseOut) {
      Serial.print(responsePrefix[deviceModel]);
      Serial.println(comBuffer);
      Serial.println();
    }
  }
  if (!(comBuffer.startsWith(STATUS_OK))) {
    if (verboseOut) {
      Serial.println("Setting pin failed!");
      delay(MENU_DELAY);
    }
    return false;
  }
  return true;
}

void HCBT::changeParity() {
  String command;

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
  } else {
    Serial.println("Invalid entry");
  }

}

bool HCBT::setParity(int parity, bool verboseOut) {
  String comBuffer = "";
  String command;

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
    Serial.print(responsePrefix[deviceModel]);
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
  testEcho(verboseOut);
  
}
