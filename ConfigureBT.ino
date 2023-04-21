/*
 * HC-05/06 AT Command Center
 * 
 *  Description: Simple HC05/06 AT configuration program. Requires 2nd UART (Serial1) defined. 
 * 
 *              Provides user menu for selecting configuration changes. Attempts to identify 
 *              HC-05/06 frame and baud settings (9600 8N1 default HC-06 settings), and firmware
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
 *    Modified: 20-Apr, 2023
 *    Revision: 1.5
 */
// uncomment following line to include debugging output to serial
#define DEBUG           1

#define BAUD_LIST_CNT   9         // count of baud rate options
#define VERS3_MIN_BAUD  3         // index for firmware 3.x minimum baud rate (4800)
#define PARITY_LIST_CNT 4         // count of UART parity options
#define FIRM_UNKNOWN    0         // index for unknown firmware 
#define FIRM_VERSION1   1         // index for firmware 1.x/2.x models
#define FIRM_VERSION3   2         // index for firmware 3.x models

#define STOP1BIT        0
#define STOP2BIT        1
#define NOPARITY        0
#define ODDPARITY       1
#define EVENPARITY      2

#define ENDLINE_NLCR    "\r\n"    // for firmware version 3
#define ENDLINE_NONE    ""        // for firmware version 1/2
#define STATUS_OK       "OK"
#define UART_CMD        "AT+UART="
#define BAUD_CMD        "AT+BAUD"

#define CONFIG_DELAY    20        // delay for basic configuration changes
#define FW1_RESPONSE    550       // for firmware version 1/2
#define FW3_RESPONSE    40        // for firmware version 3
#define BITS_PER_CHAR   12        // UART frames - worst case: parity, 2 stop bits

/* line ending to be applied to AT commands according to firmware version # */
int firmVersion = FIRM_UNKNOWN;
#define VERSION_KNOWN   (firmVersion != FIRM_UNKNOWN)

//String lineEnding = ENDLINE_NLCR;
char charFromBT;
int baudRate = VERS3_MIN_BAUD;
int parity = NOPARITY + 1;
int stopBits = STOP1BIT;
int selection;
//bool found = false;
String hc06Version = "";
String nameBT, pin, command;

const unsigned long baudRateList[] = {0, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
const uint8_t parityList[] = {0, SERIAL_8N1, SERIAL_8O1, SERIAL_8E1};
const String parityType[] = {"Invalid", "None", "Odd", "Even"};
const String parityCmd[] = {"Invalid", "AT+PN", "AT+PO", "AT+PE"};
const String parityParam[] = {"Invalid", ",0\r\n", ",1\r\n", ",2\r\n"};
const String stopParam[] = {"", ",0", ",1"};
const String lineEnding[] = {"", "", "\r\n"};
const unsigned long responseMS[] = {1, FW1_RESPONSE, FW3_RESPONSE}
enum HC06commands {LINE_END = 0,
                  ECHO,
                  HCVERSION,
                  BTNAME,
                  BTPIN,
                  UART_GET,
                  BAUD_SET,
                  PARITY_SET,
                  OTHER_CMD};
                  
const String atCommands[][2] = {{"", "\r\n"},
                              {"AT", "AT\r\n"},
                              {"AT+VERSION", "AT+VERSION?\r\n"},
                              {"AT+NAME", "AT+NAME="},
                              {"AT+PIN", "AT+PSWD="}};
//                              {"AT+BAUD", "AT+UART="},
//                              {"AT+PN", "AT+UART="},
//                              {"AT+PE", "AT+UART="},
//                              {"AT+PO", "AT+UART="}};

// Worst-case count of expected characters for response to commands.
//  Indexed based on HC06commands values.
const int responseChars[] = {0,
                            4,
                            26,
                            22,
                            6,
                            22,
                            8,
                            8,
                            40};

void setup() {
  Serial.begin(57600);
  delay(1000);
  Serial1.begin(baudRateList[baudRate], parityList[parity]);
  delay(CONFIG_DELAY); 
//  Serial.println("For accurate operation, set 'No line ending' in Serial Monitor setting (lower status bar)");
//  delay(100);
  Serial.flush();
  printMenu();
}

void loop() {
  while (Serial1.available() > 0) {
    Serial.print("[HC06]: ");
    Serial.println(Serial1.readString());
    Serial.println();
  }

  if (Serial.available() > 0) {
    command = Serial.readString();
//    selection = command.toInt();
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

void responseDelay(unsigned long characters, int firmware, HC06commands command) {
  if (baudRate == 0) return;
  unsigned long writeMS = (characters + responseChars[command]) * BITS_PER_CHAR * 1000 / baudRateList[baudRate];
  delay(writeMS + responseMS[firmware]);
}

void clearInputStream() {
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
//  lineEnding = ENDLINE_NLCR;
//  baudRate = VERS3_MIN_BAUD;
//  parity = 1;
  firmVersion = FIRM_UNKNOWN;
//  found = false;
  hc06Version = "";
  
  Serial1.end();
  delay(CONFIG_DELAY);
  Serial.print("Searching for firmware and version of HC06");
  // TODO  change both inner loops to for loops - found loop needed?
  for (int firmware = FIRM_VERSION3; firmware > FIRM_UNKNOWN; firmware--) {
    for (parity = 1; parity < PARITY_LIST_CNT; parity++) {
      // firmware version 3.x does not support baud rate below 4800
      if (firmware == FIRM_VERSION3) {
        baudRate = VERS3_MIN_BAUD;
      } else {
        baudRate = 1;
      }
      for ( ; baudRate < BAUD_LIST_CNT; baudRate++) {
        // Test for Version x.x firmware AT echo
        command = atCommands[ECHO][firmware - 1]; // test connection
#ifdef DEBUG
        // debugging instructions to verify characters sent to UART
        Serial.print("Command length: ");
        Serial.println(command.length());
        for (int i = 0; i < command.length(); i++) {
          Serial.print("\t");
          Serial.print(command.charAt(i), HEX);
        }
        Serial.println();
#endif
        Serial.print(" .");
        Serial1.begin(baudRateList[baudRate], parityList[parity]);
        delay(CONFIG_DELAY);
        clearInputStream();
        Serial1.print(command);
        Serial1.flush();
        responseDelay(command.length(), firmware, ECHO);
        if (Serial1.available() > 0) {
          comBuffer = Serial1.readString();
#ifdef DEBUG
          Serial.println();
          Serial.println(comBuffer);
          for (int i = 0; i < comBuffer.length(); i++) {
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
    command = atCommands[HCVERSION][firmVersion - 1];  // test connection
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
  baudRate = Serial.parseInt();
  if (baudRate == 0) {
    Serial.println("Canceled");
  } else if(baudRate < BAUD_LIST_CNT) {
    Serial1.end();
    Serial1.begin(baudRateList[baudRate], parityList[parity]);
    Serial.print("Set local baud rate to ");
    Serial.println(baudRateList[baudRate]);
    Serial.println();
    command = "AT" + lineEnding[firmVersion];   // send echo to determine if current setting matches device
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
  Serial.println("It is advised that parity is left at same setting as found hardware.");
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
    command = "AT" + lineEnding[firmVersion];   // send echo to determine if current setting matches device
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
  command = atCommands[HCVERSION][firmVersion - 1];
  clearInputStream();
  Serial1.print(command);
  Serial1.flush();
  responseDelay(command.length(), firmVersion, HCVERSION);
  while (Serial1.available() > 0) {
    Serial.print("[HC06]: ");
    Serial.println(Serial1.readString());
    Serial.println();
  }
}

String constructUARTstring(int baud, int prty, int stops) {
  return String(UART_CMD) + baudRateList[baud] + "," + stops + "," + (prty-1) + lineEnding[FIRM_VERSION3];
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
  int tempBaud = command.toInt();
  if (tempBaud == 0) {
    Serial.println("Canceled");
  } else if (tempBaud < BAUD_LIST_CNT) {
    if (firmVersion == FIRM_VERSION3) {
      if (tempBaud < VERS3_MIN_BAUD) {
        Serial.println("Baud rates below 4800 not supported by this firmware.");
        return;
      }
      command = constructUARTstring(tempBaud, parity, stopBits);
    } else {
      command = String(BAUD_CMD) + tempBaud + lineEnding[firmVersion];
    }
    Serial.print("Setting HC06 and local baud rate to ");
    Serial.println(baudRateList[tempBaud]);
    Serial.println("\tsending command: " + command);
    Serial.println();
    clearInputStream();
    Serial1.print(command);
    Serial1.flush();
    responseDelay(command.length(), firmVersion, BAUD_SET);
    if (Serial1.available() > 0) {
      comBuffer = Serial1.readString();
      Serial.print("[HC06]: ");
      Serial.println(comBuffer);
      Serial.println();
#ifdef DEBUG
      for (int i = 0; i < comBuffer.length(); i++) {
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
    command = atCommands[BTNAME][firmVersion - 1] + nameBT + lineEnding[firmVersion];
    Serial.println("\tsending command: " + command);
    clearInputStream();
    Serial1.print(command);
    Serial1.flush();
    responseDelay(command.length(), firmVersion, BTNAME);
    while (Serial1.available() > 0) {
      comBuffer = Serial1.readString();
      Serial.print("[HC06]: ");
      Serial.println(comBuffer);
      Serial.println();
#ifdef DEBUG
      for (int i = 0; i < comBuffer.length(); i++) {
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
    for (int i = 0; i < 4; i++) {
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
    for (int i = 0; i < pin.length(); i++) {
      Serial.print(pin[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
#endif
    return;
  }

  Serial.print("Setting pin to ");
  Serial.println(pin);
  command = atCommands[BTPIN][firmVersion - 1] + pin + lineEnding[firmVersion];
  Serial.println("\tsending command: " + command);
  clearInputStream();
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
  int tempParity = command.toInt();
//  parity = Serial.parseInt();
  if (tempParity == 0) {
    Serial.println("Canceled");
  } else if (tempParity > 3) {
    Serial.println("Invalid entry");
  } else {
    if (firmVersion == FIRM_VERSION3) {
      command = constructUARTstring(baudRate, tempParity, stopBits);
    } else {
      command = parityCmd[tempParity];
    }
    Serial.println("Setting to " + parityType[tempParity] + " Parity check");
    Serial.println("\tsending command: " + command);
    clearInputStream();
    Serial1.print(command);
    Serial1.flush();
    responseDelay(command.length(), firmVersion, PARITY_SET);

    if (Serial1.available() > 0) {
      comBuffer = Serial1.readString();
      Serial.print("[HC06]: ");
      Serial.println(comBuffer);
      Serial.println();
#ifdef DEBUG
      for (int i = 0; i < comBuffer.length(); i++) {
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
  }
  
}
