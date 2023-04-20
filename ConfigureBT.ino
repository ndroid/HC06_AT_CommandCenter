/*
 * HC-06 AT Command Center
 * 
 *  Description: Simple HC06 AT configuration program. Requires 2nd UART (Serial1) defined. 
 * 
 *              Provides user menu for selecting configuration changes. Attempts to identify 
 *              HC-06 frame and baud settings (9600 8N1 default HC-06 settings), and firmware
 *              version. NL+CR required by new firmware, but not older firmware. Arduino device
 *              UART settings can be changed to match HC-06 through program. HC-06 must be 
 *              in configuration mode (AT mode) (LED blinking to indicate Not Connected).
 *              
 *              Recent batches of HC-06 appear to have HC-05 firmware (reporting Version 3).
 *              There is no documentation of a Version 3 firmware for HC-06. AT commands differ
 *              for HC-05 firmware. Support will be added for these devices in Revision 2 of
 *              this software.
 *              
 *              AT response delays:
 *                Around 10~25ms for Version 3.x  (newline terminated) - max observed 35ms
 *                Around 500ms for Version 1.x    (timeout terminated) - max observed 525ms
 *                Serial writes are asynchronous, so delays must also consider write time
 *                
 *    HC06 connections:
 *      TXD <--> [Serial 1 RX] (pin 19 on Mega)(pin 13 on MKR WiFi board) 
 *      RXD <--> [Serial 1 TX] (pin 18 on Mega)(pin 14 on MKR WiFi board)
 *      
 *  Created on: 18-Oct, 2021
 *      Author: miller4@rose-hulman.edu
 *    Modified: 18-Apr, 2023
 *    Revision: 1.5
 */
 
#define BAUD_LIST_CNT   9         // count of baud rate options
#define PARITY_LIST_CNT 4         // count of UART parity options
#define FIRM_VERSION1   0         // index for firmware 1.x/2.x models
#define FIRM_VERSION3   1         // index for firmware 3.x models

#define STOP1BIT        0
#define STOP2BIT        1
#define NOPARITY        0
#define ODDPARITY       1
#define EVENPARITY      2

#define ENDLINE_NLCR    "\r\n"    // for firmware version 3
#define ENDLINE_NONE    ""        // for firmware version 1/2
#define STATUS_OK       "OK"

/* line ending to be applied to AT commands according to firmware version # */
int firmVersion = FIRM_VERSION1;
//String lineEnding = ENDLINE_NLCR;
char charFromBT;
int baudRate = 1;
int parity = 1;
int selection;
bool found = false;
String hc06Version = "";
String nameBT, pin, command;

const unsigned long baudRateList[] = {0, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
const uint8_t parityList[] = {0, SERIAL_8N1, SERIAL_8E1, SERIAL_8O1};
const String parityType[] = {"Invalid", "None", "Odd", "Even"};
const String parityParam[] = {"Invalid", ",0\r\n", ",1\r\n", ",2\r\n"};
const String stopParam[] = {"", ",0", ",1"};
const String lineEnding[] = {"", "\r\n"};

enum HC06commands {LINE_END = 0,
                  ECHO,
                  HCVERSION,
                  BTNAME,
                  BTPIN,
                  HCBAUD,
                  NO_PARITY,
                  EVEN_PARITY,
                  ODD_PARITY};
                  
const String atCommands[][2] = {{"", "\r\n"},
                              {"AT", "AT\r\n"},
                              {"AT+VERSION", "AT+VERSION?\r\n"},
                              {"AT+NAME", "AT+NAME="},
                              {"AT+PIN", "AT+PSWD="},
                              {"AT+BAUD", "AT+UART="},
                              {"AT+PN", "AT+UART="},
                              {"AT+PE", "AT+UART="},
                              {"AT+PO", "AT+UART="}};

void setup() {
  Serial.begin(57600);
  delay(1000);
  Serial1.begin(baudRateList[baudRate], parityList[parity]);
  delay(100); 
  Serial.println("For accurate operation, set 'No line ending' in Serial Monitor setting (lower status bar)");
  delay(100);
  Serial.flush();
  scanDevice();
  printMenu();
}

void loop() {
  while (Serial1.available() > 0) {
    Serial.print("[HC06]: ");
    Serial.println(Serial1.readString());
    Serial.println();
  }

  if (Serial.available() > 0) {
    selection = Serial.parseInt();
    switch(selection) {
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

void printMenu() {
  Serial.println();
  Serial.println();
  Serial.write(12);   // Form feed (not supported in Serial Monitor)
  if (found) {
    Serial.print("Device found - Version: ");
    Serial.println(hc06Version);
    Serial.print("Current baud rate: ");
    Serial.println(baudRateList[baudRate]);
    Serial.print("Current parity: ");
    Serial.println(parityType[parity]);
  } else {
    Serial.println("Device version/configuration unknown.");
    Serial.println("Check connections and select option 8 to scan for device.");
  }
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

void scanDevice() {
  String comBuffer;
//  lineEnding = ENDLINE_NLCR;
  baudRate = 1;
  parity = 1;
  found = false;
  hc06Version = "";
  
  Serial1.end();
  Serial.print("Searching for firmware and version of HC06");
  while (!found) {
    // TODO  change both inner loops to for loops - found loop needed?
    while (parity < PARITY_LIST_CNT) {
      while (baudRate < BAUD_LIST_CNT) {
        // Test for Version 1.x firmware AT echo
        firmVersion = FIRM_VERSION1;
        command = atCommands[ECHO][firmVersion];  // test connection
#ifdef DEBUG
        // debugging instructions to verify characters sent to UART
        Serial.print("Command length: ");
        Serial.println(command.length());
        for (int i = 0; i < command.length(); i++) {
          Serial.print(command.charAt(i), HEX);
          Serial.print("\t");
        }
        Serial.println();
#endif
        Serial.print(" .");
        Serial1.begin(baudRateList[baudRate], parityList[parity]);
        delay(100); 
        while (Serial1.available() > 0) {
          // wait until input stream is clear
          Serial1.read();
        }
        Serial1.print(command);
        Serial1.flush();
        delay(1000); 
        if (Serial1.available() > 0) {
          comBuffer = Serial1.readString();
          Serial.println();
          Serial.println(comBuffer);
#ifdef DEBUG
          // debugging instructions to verify characters received over UART
          for (int i = 0; i < comBuffer.length(); i++) {
            Serial.print(comBuffer.charAt(i), HEX);
            Serial.print("\t");
          }
          Serial.println();
#endif
          if (comBuffer.startsWith(STATUS_OK)) {
            found = true;
            break;
          }
        }
        while (Serial1.available() > 0) {
          // wait until input stream is clear
          Serial1.read();
        }
        // end Test for Version 1.x firmware
        
        // Test for Version 3.x firmware AT echo
        firmVersion = FIRM_VERSION3;
        command = atCommands[ECHO][firmVersion];  // test connection
#ifdef (DEBUG)
        // debugging instructions to verify characters sent to UART
        Serial.print("Command length: ");
        Serial.println(command.length());
        for (int i = 0; i < command.length(); i++) {
          Serial.print(command.charAt(i), HEX);
          Serial.print("\t");
        }
        Serial.println();
#endif
        // ensure HC06 is not waiting for termination of partially complete command
        delay(100); 
        Serial1.print(lineEnding[FIRM_VERSION3]);
        Serial1.flush();
        delay(500); 
        while (Serial1.available() > 0) {
          // wait until input stream is clear
          Serial1.read();
        }
        Serial1.print(command);
        Serial1.flush();
        delay(1000); 
        if (Serial1.available() > 0) {
          comBuffer = Serial1.readString();
          Serial.println();
          Serial.println(comBuffer);
#ifdef DEBUG
          // debugging instructions to verify characters received over UART
          for (int i = 0; i < comBuffer.length(); i++) {
            Serial.print(comBuffer.charAt(i), HEX);
            Serial.print("\t");
          }
          Serial.println();
#endif
          if (comBuffer.startsWith(STATUS_OK)) {
            found = true;
            break;
          }
        }
        while (Serial1.available() > 0) {
          // wait until input stream is clear
          Serial1.read();
        }
        // end Test for Version 3.x firmware
        Serial1.end();
        baudRate++;
        delay(150); 
      }
      if (found) break;
      baudRate = 1;
      parity++;
    }
    if (found) break;
  }
  Serial.println();

  if (found) {
    command = atCommands[HCVERSION][firmVersion];  // test connection
    Serial1.print(command);
    Serial1.flush();
    delay(1000); 
    if (Serial1.available() > 0) {
      hc06Version = Serial1.readString();
    }
  }
  
}

void setLocalBaud() {
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
  baudRate = Serial.parseInt();
  if (baudRate == 0) {
    Serial.println("Canceled");
  } else if(baudRate < BAUD_LIST_CNT) {
    Serial1.end();
    Serial1.begin(baudRateList[baudRate], parityList[parity]);
    Serial.print("Set local baud rate to ");
    Serial.println(baudRateList[baudRate]);
    Serial.println();
    command = "AT" + lineEnding;   // send echo to determine if current setting matches device
    Serial1.print(command);
    Serial1.flush();
    delay(1000); 
    while (Serial1.available() > 0) {
      Serial.print("[HC06]: ");
      Serial.println(Serial1.readString());
      Serial.println();
    }
  } else {
    Serial.println("Invalid entry");
  }
  
}

void setLocalParity() {
  Serial.print("Current parity: ");
  Serial.println(parityType[parity]);
  Serial.println("Select parity option:");
  Serial.println("\t(0) Cancel");
  Serial.println("\t(1).......No parity");
  Serial.println("\t(2).......Odd parity");
  Serial.println("\t(3).......Even parity");

  while (Serial.available() < 1);
  parity = Serial.parseInt();
  if (parity == 0) {
    Serial.println("Canceled");
  } else {
    switch(parity) {
      case 1:
        Serial1.end();
        Serial1.begin(baudRateList[baudRate], parityList[parity]);
        Serial.println("Setting to No Parity check");
        break;
      case 2:
        Serial1.end();
        Serial1.begin(baudRateList[baudRate], parityList[parity]);
        Serial.println("Setting to Odd Parity check");
        break;
      case 3:
        Serial1.end();
        Serial1.begin(baudRateList[baudRate], parityList[parity]);
        Serial.println("Setting to Even Parity check");
        break;
      default:
        Serial.println("Invalid entry");
    }
    Serial.println();
    command = "AT" + lineEnding;   // send echo to determine if current setting matches device
    Serial1.print(command);
    Serial1.flush();
    delay(1000); 
    while (Serial1.available() > 0) {
      Serial.print("[HC06]: ");
      Serial.println(Serial1.readString());
      Serial.println();
    }
  }
  
}

void getVersion() {
  command = "AT+VERSION" + lineEnding[firmVersion];
  Serial1.print(command);
  Serial1.flush();
  delay(1000); 
  while (Serial1.available() > 0) {
    Serial.print("[HC06]: ");
    Serial.println(Serial1.readString());
    Serial.println();
  }
  
}

void setBaudRate() {
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
  baudRate = Serial.parseInt();
  if (baudRate == 0) {
    Serial.println("Canceled");
  } else if(baudRate < BAUD_LIST_CNT) {
    command = String("AT+BAUD") + baudRate + lineEnding[firmVersion];
    Serial.print("Setting HC06 and local baud rate to ");
    Serial.println(baudRateList[baudRate]);
    Serial.println("sending command:" + command);
    Serial.println();
    Serial1.print(command);
    Serial1.flush();
    Serial1.end();
    Serial1.begin(baudRateList[baudRate], parityList[parity]);
    delay(1000); 
    while (Serial1.available() > 0) {
      Serial1.readString();   // clear buffer
      delay(100); 
    }
    // repeat command since response may be missed while changing baud rate
    Serial.println("sending command:" + command);
    Serial1.print(command);
    Serial1.flush();
    delay(1000); 
    while (Serial1.available() > 0) {
      Serial.print("[HC06]: ");
      Serial.println(Serial1.readString());
      Serial.println();
    }
  } else {
    Serial.println("Invalid entry");
  }
  
}

void setName() {
  Serial.println("Enter BT name (max 15 characters): ");

  while (Serial.available() < 1);
  nameBT = Serial.readString();
  if (nameBT.length() > 0) {
    nameBT = "HC06_" + nameBT.substring(0, 15);
    Serial.print("Setting name to ");
    Serial.println(nameBT);
    command = String("AT+NAME=") + nameBT + lineEnding[firmVersion];
    Serial.println("sending command:" + command);
    Serial1.print(command);
    Serial1.flush();
    delay(1000); 
    while (Serial1.available() > 0) {
      Serial.print("[HC06]: ");
      Serial.println(Serial1.readString());
      Serial.println();
    }
  } else {
    Serial.println("Invalid entry (empty string)");
  }
  
}

void setPin() {
  Serial.println("Enter pin number (4 digits): ");

  while (Serial.available() < 1);
  pin = Serial.readString();
  if ((pin.length() == 4) && (pin.toInt() != 0)) {
    Serial.print("Setting pin to ");
    Serial.println(pin);
    command = String("AT+PIN") + pin + lineEnding[firmVersion];
    Serial.println("sending command:" + command);
    Serial1.print(command);
    Serial1.flush();
    delay(1000); 
    while (Serial1.available() > 0) {
      Serial.print("[HC06]: ");
      Serial.println(Serial1.readString());
      Serial.println();
    }
  } else {
    Serial.println("Invalid entry (not 4-digit integer)");
    Serial.print("Characters: ");
    for (int i = 0; i < pin.length(); i++) {
      Serial.print(pin[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
  
}

void setParity() {
  Serial.print("Current parity: ");
  Serial.println(parityType[parity]);
  Serial.println("Select parity option:");
  Serial.println("\t(0) Cancel");
  Serial.println("\t(1).......No parity");
  Serial.println("\t(2).......Odd parity");
  Serial.println("\t(3).......Even parity");

  while (Serial.available() < 1);
  parity = Serial.parseInt();
  if (parity == 0) {
    Serial.println("Canceled");
  } else {
    switch(parity) {
      case 1:
        command = "AT+PN" + lineEnding[firmVersion];
        Serial.println("Setting to No Parity check");
        Serial.println("sending command:" + command);
        Serial1.print(command);
        Serial1.flush();
        Serial1.end();
        Serial1.begin(baudRateList[baudRate], parityList[parity]);
        break;
      case 2:
        command = "AT+PO" + lineEnding[firmVersion];
        Serial.println("Setting to Odd Parity check");
        Serial.println("sending command:" + command);
        Serial1.print(command);
        Serial1.flush();
        Serial1.end();
        Serial1.begin(baudRateList[baudRate], parityList[parity]);
        break;
      case 3:
        command = "AT+PE" + lineEnding[firmVersion];
        Serial.println("Setting to Even Parity check");
        Serial.println("sending command:" + command);
        Serial1.print(command);
        Serial1.flush();
        Serial1.end();
        Serial1.begin(baudRateList[baudRate], parityList[parity]);
        break;
      default:
        command = "AT" + lineEnding[firmVersion];     // to avoid error in sending command below
        Serial.println("Invalid entry");
    }
    if ((command.length() - lineEnding[firmVersion].length()) > 2) {
      Serial.println("To complete change of parity, remove then reconnect power to HC-06.");
      Serial.println("Enter any character when complete (LED should be blinking).");
      while (Serial.available() < 1);
      Serial.readString();   // clear buffer
//            delay(1000); 
      while (Serial1.available() > 0) {
        Serial1.readString();   // clear buffer
        delay(100); 
      }
      // send echo command to verify device at new baud rate
      command = "AT" + lineEnding[firmVersion];
      Serial.println("Verifying completion - sending command:" + command);
      Serial1.print(command);
      Serial1.flush();
      delay(1000); 
      found = false;
      if (Serial1.available() > 0) {
        if ((Serial1.readString()).startsWith(STATUS_OK)) {
          Serial.println("[HC06]: OK");
          found = true;
          return;
        }
      }
      Serial.println("Confirmation not received");
    }
  }
  
}
