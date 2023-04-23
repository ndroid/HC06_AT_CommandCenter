/*
 * HC-05/06 AT Command Center
 *  Version 2.0
 * 
 *  Description: Simple HC05/06 AT configuration program. Requires 2nd UART (Serial1) defined. 
 * 
 *              Provides user menu for selecting configuration changes. Attempts to identify 
 *              HC-05/06 frame and baud settings (9600 8N1 default HC-06 settings), and firmware
 *              version. Serial1 automatically configured to match HC-06 UART settings. HC-05/06 
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
 *    HC06 connections:
 *                              pin/board   Mega    MKR   Uno WiFi  Zero    Due
 *      TXD <--> [Serial 1 RX]               19      13      0        0      19
 *      RXD <--> [Serial 1 TX]               18      14      1        1      18
 *      
 *  Created on: 18-Oct, 2021
 *      Author: miller4@rose-hulman.edu
 *    Modified: 22-Apr, 2023
 *    Revision: 2.0
 */
// uncomment following line to include debugging output to serial
//#define DEBUG           1

#include "configureBT.h"

void setup() {
  // configure Serial Monitor UART (57600 8N1)
  Serial.begin(57600);
  delay(1000);
  // configure Serial1 UART interface to HC-xx
  Serial1.begin(baudRateList[baudRate], parityList[parity]);
  delay(CONFIG_DELAY); 
  Serial.flush();
  printMenu();
}

void loop() {
  // check for any messages from HC-xx UART
  while (Serial1.available() > 0) {
    Serial.print("[HC06]: ");
    Serial.println(Serial1.readString());
    Serial.println();
  }

  // check for user selection of menu option
  if (Serial.available() > 0) {
    command = Serial.readString();
    switch(command.toInt()) {
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
    
    printMenu();
  }
  
  delay(100);
}

/*
 * responseDelay
 *  
 * Delays for period necessary to allow completion of HC-xx response to command.
 * 
 *  characters  - count of characters in AT command
 *  firmware    - firmware version identifier for HC-xx
 *  command     - index of AT command (as defined in HC06commands)
 */
void responseDelay(unsigned long characters, int firmware, HC06commands command) {
  if ((baudRate < 0) || (baudRate >= BAUD_LIST_CNT)) return;
  unsigned long writeMS = (characters + responseChars[command]) * BITS_PER_CHAR * 1000 
                              / baudRateList[baudRate];
  delay(writeMS + responseMS[firmware]);
}

void clearInputStream(int firmware) {
  if (firmware == FIRM_VERSION3) {
    // ensure HC06 is not waiting for termination of partially complete command
    Serial1.print(lineEnding[FIRM_VERSION3]);
    Serial1.flush();
    delay(FW3_RESPONSE);
  }
  while (Serial1.available() > 0) {
    // wait until input stream is clear
    Serial1.read();
  }
}

void printMenu() {
  while (firmVersion == FIRM_UNKNOWN) {
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
  Serial.println("\t(1) Set HC06 Baud Rate");
  Serial.println("\t(2) Set HC06 BT name");
  Serial.println("\t(3) Set HC06 BT pin");
  Serial.println("\t(4) Set HC06 parity");
  Serial.println("\t(5) Set local Baud Rate (for testing only)");
  Serial.println("\t(6) Set local parity (for testing only)");
  Serial.println("\t(7) Get version (useful to verify connection/baud)");
  Serial.println("\t(8) Rescan HC06 device");
  
}

bool scanDevice() {
  String comBuffer;
  firmVersion = FIRM_UNKNOWN;
  hc06Version = "";
  
  Serial1.end();
  delay(CONFIG_DELAY);
  Serial.print("Searching for firmware and version of HC06");
  // Scan through possible UART configurations for each firmware version. 
  //  Use AT command to test for OK response.
  for (int firmware = FIRM_VERSION3; firmware > FIRM_UNKNOWN; firmware--) {
    for (parity = NOPARITY; parity < PARITY_LIST_CNT; parity++) {
      // firmware version 3.x does not support baud rate below 4800
      if (firmware == FIRM_VERSION3) {
        baudRate = VERS3_MIN_BAUD;
      } else {
        baudRate = 0;
      }
      for ( ; baudRate < BAUD_LIST_CNT; baudRate++) {
        // Test for Version x.x firmware AT echo
        command = atCommands[ECHO][firmware - 1]; // test connection
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
          if (comBuffer.startsWith(STATUS_OK)) {
            firmVersion = firmware;
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
      if (firmVersion != FIRM_UNKNOWN)  break;
    } // end parity loop
    if (firmVersion != FIRM_UNKNOWN)  break;
  } // end firmware loop
  Serial.println();

  if (firmVersion != FIRM_UNKNOWN) {
    command = atCommands[HCVERSION][firmVersion];  // test connection
    Serial1.print(command);
    Serial1.flush();
    responseDelay(command.length(), firmVersion, HCVERSION);
    if (Serial1.available() > 0) {
      hc06Version = Serial1.readString();
    }
  }
  
  return (firmVersion != FIRM_UNKNOWN);
}

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
    Serial.println("Testing new parity configuration - request firmware version . . .");
    getVersion();
  } else {
    Serial.println("Invalid entry");
  }
  
}

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
    Serial.println("Testing new parity configuration - request firmware version . . .");
    getVersion();
  } else {
    Serial.println("Invalid entry");
  }
  
}

void getVersion() {
  String comBuffer = "";
  command = atCommands[HCVERSION][firmVersion];
  clearInputStream(firmVersion);
  Serial1.print(command);
  Serial1.flush();
  responseDelay(command.length(), firmVersion, HCVERSION);
  if (Serial1.available() > 0) {
    comBuffer = Serial1.readString();
    Serial.print("[HC06]: ");
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
  if (!(comBuffer.startsWith(STATUS_OK))) {
    Serial.println("OK response not received.");
    firmVersion = FIRM_UNKNOWN;
  }
}

String constructUARTstring(int baud, int prty, int stops) {
  return String(UART_CMD) + baudRateList[baud] + "," + stops + "," 
            + (prty) + lineEnding[FIRM_VERSION3];
}

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

  while (Serial.available() < 1);
  command = Serial.readString();
  int tempBaud = command.toInt() - 1;
  if (tempBaud < 0) {
    Serial.println("Canceled");
  } else if (tempBaud < BAUD_LIST_CNT) {
    if (firmVersion == FIRM_VERSION3) {
      if (tempBaud < VERS3_MIN_BAUD) {
        Serial.println("Baud rates below 4800 not supported by this firmware.");
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
      Serial.println("Request failed.");
      return;
    }
    Serial1.end();
    baudRate = tempBaud;
    delay(CONFIG_DELAY);
    Serial1.begin(baudRateList[baudRate], parityList[parity]);
    delay(CONFIG_DELAY);
    Serial.println("Testing new baud rate configuration - request firmware version . . .");
    getVersion();
  } else {
    Serial.println("Invalid entry");
  }
  
}

void setName() {
  String comBuffer = "";
  Serial.println("Enter BT name (max 15 characters - prepends HC06_): ");

  while (Serial.available() < 1);
  nameBT = Serial.readString();
  nameBT.trim();  // remove leading or trailing whitespaces (newline characters)
  if (nameBT.length() > 0) {
    nameBT = "HC06_" + nameBT.substring(0, 15);
    Serial.print("Setting name to ");
    Serial.println(nameBT);
    command = atCommands[BTNAME][firmVersion] + nameBT + lineEnding[firmVersion];
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
  
}

void setPin() {
  String pin;
  if (firmVersion == FIRM_VERSION3) {
    Serial.println("Enter new BT passkey (14 characters max): ");
  } else {
    Serial.println("Enter new pin number (4 digits): ");
  }

  while (Serial.available() < 1);
  pin = Serial.readString();
  pin.trim();
  if (firmVersion == FIRM_VERSION3) {
    // version 3 FW appears to require quotes around passkey,
    //  though this isn't indicated in documentation
    //  https://forum.arduino.cc/t/password-hc-05/481294
    pin = String("\"") + pin.substring(0, 14) + String("\"");
  } else if (pin.length() == 4) {
    for (unsigned int i = 0; i < 4; i++) {
      if (!isDigit(pin.charAt(i))) {
        Serial.println("Invalid entry (not 4-digit integer)");
#ifdef DEBUG
        Serial.print("\tCharacters: ");
        for (i = 0; i < pin.length(); i++) {
          Serial.print(pin[i], HEX);
          Serial.print(" ");
        }
        Serial.println();
#endif
        return;
      }
    }
  } else {
    Serial.println("Invalid entry (not 4-digit integer)");
#ifdef DEBUG
    Serial.print("\tCharacters: ");
    for (unsigned int i = 0; i < pin.length(); i++) {
      Serial.print(pin[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
#endif
    return;
  }

  Serial.print("Setting pin to ");
  Serial.println(pin);
  command = atCommands[BTPIN][firmVersion] + pin + lineEnding[firmVersion];
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
}

void setParity() {
  String comBuffer = "";
  Serial.print("Current parity: ");
  Serial.println(parityType[parity]);
  Serial.println("Select parity option:");
  Serial.println("\t(0) Cancel");
  Serial.println("\t(1).......No parity");
  Serial.println("\t(2).......Odd parity");
  Serial.println("\t(3).......Even parity");

  while (Serial.available() < 1);
  command = Serial.readString();
  int tempParity = command.toInt() - 1;
  if (tempParity < 0) {
    Serial.println("Canceled");
  } else if (tempParity < PARITY_LIST_CNT) {
    if (firmVersion == FIRM_VERSION3) {
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
      Serial.println("Request failed.");
      return;
    }
    Serial1.end();
    parity = tempParity;
    delay(CONFIG_DELAY);
    if (firmVersion == FIRM_VERSION1) {
      Serial.println("To complete change of parity, remove then reconnect power to HC-06.");
      Serial.println("Enter any character when complete (LED should be blinking).");
      while (Serial.available() < 1);
      Serial.readString();   // clear buffer
    }
    Serial1.begin(baudRateList[baudRate], parityList[parity]);
    delay(CONFIG_DELAY);
    Serial.println("Testing new parity configuration - request firmware version . . .");
    getVersion();
  } else {
    Serial.println("Invalid entry");
  }
  
}
