/*
 * HC-06 AT Command Center
 * 
 * Description: Simple HC06 configuration program. Requires 2nd UART (Serial1) defined. 
 *              Provides user menu for selecting configuration changes. Initial Serial1
 *              settings are 9600 8N1 (matches default HC-06 settings). Arduino device
 *              UART settings can be changed to match HC-06 through program. HC-06 must
 *              be in configuration mode (LED blinking to indicate Not Connected). 
 *              NL+CR required by new firmware, but not older firmware. 
 *              
 *    HC06 connections:
 *      TXD <--> pin 3 (Serial 1 RX)
 *      RXD <--> pin 4 (Serial 1 TX)
 *      
 *  Created on: 18-Oct, 2021
 *      Author: miller4@rose-hulman.edu
 *    Modified: 7-Feb, 2022
 *    Revision: 1.3
 */
 
#define BAUD_LIST_CNT   9       // count of baud rate options
#define ENDLINE_NLCR    "\n\r"  // newline terminator for version 3

/* line ending to be applied to AT commands according to firmware version # */
String lineEnding = ENDLINE_NLCR;
char charFromBT;
int baudRate = 4;   // initial baud rate setting (default 9600)
int parity = 1;     // initial parity setting (default none)
int selection;      // selection value from menu
String nameBT, pin, command;

unsigned long baudRateList[] = {0, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
uint8_t parityList[] = {0, SERIAL_8N1, SERIAL_8E1, SERIAL_8O1};
String parityType[] = {"Invalid", "None", "Even", "Odd"};

void setup() {
  Serial.begin(57600);
  delay(4000); // allow device to pair after reset or power up 
  Serial1.begin(baudRateList[baudRate], parityList[parity]);
  delay(100); 
  //Serial1.println("Ready to receive characters over BT");
  Serial.println("For accurate operation, set 'No line ending' in Serial Monitor setting (lower status bar)");
  Serial.println("Sending AT command to HC06. Should respond with OK if frame settings match . . .");
  Serial1.print("AT" + lineEnding);  // test connection
  Serial1.flush();
  delay(1000); 
  while (Serial1.available() > 0) {
    Serial.print("[HC06]: ");
    Serial.println(Serial1.readString());
    Serial.println();
  }
  Serial.println("Select option:");
  Serial.println("\t(1) Set local Baud Rate (initially 9600)");
  Serial.println("\t(2) Set local parity (initially none)");
  Serial.println("\t(3) Get version (useful to verify connection/baud)");
  Serial.println("\t(4) Set HC06 Baud Rate");
  Serial.println("\t(5) Set HC06 BT name");
  Serial.println("\t(6) Set HC06 BT password");
  Serial.println("\t(7) Set HC06 parity");
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
        break;
      case 2: 
        Serial.print("Current parity: ");
        Serial.println(parityType[parity]);
        Serial.println("Select parity option:");
        Serial.println("\t(0) Cancel");
        Serial.println("\t(1).......No parity");
        Serial.println("\t(2).......Even parity");
        Serial.println("\t(3).......Odd parity");

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
              Serial.println("Setting to Even Parity check");
              break;
            case 3:
              Serial1.end();
              Serial1.begin(baudRateList[baudRate], parityList[parity]);
              Serial.println("Setting to Odd Parity check");
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
        break;
      case 3: 
        command = "AT+VERSION" + lineEnding;
        Serial1.print(command);
        Serial1.flush();
        delay(1000); 
        while (Serial1.available() > 0) {
          Serial.print("[HC06]: ");
          Serial.println(Serial1.readString());
          Serial.println();
        }
        break;
      case 4:
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
          command = String("AT+BAUD") + baudRate + lineEnding;
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
        break;
      case 5: 
        Serial.println("Enter BT name (max 15 characters): ");

        while (Serial.available() < 1);
        nameBT = Serial.readString();
        if (nameBT.length() > 0) {
          nameBT = "HC06_" + nameBT.substring(0, 15);
          Serial.print("Setting name to ");
          Serial.println(nameBT);
          command = String("AT+NAME") + nameBT + lineEnding;
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
        break;
      case 6: 
        Serial.println("Enter pin number (4 digits): ");

        while (Serial.available() < 1);
        pin = Serial.readString();
        if ((pin.length() == 4) && (pin.toInt() != 0)) {
          Serial.print("Setting pin to ");
          Serial.println(pin);
          command = String("AT+PIN") + pin + lineEnding;
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
        break;
      case 7: 
        Serial.print("Current parity: ");
        Serial.println(parityType[parity]);
        Serial.println("Select parity option:");
        Serial.println("\t(0) Cancel");
        Serial.println("\t(1).......No parity");
        Serial.println("\t(2).......Even parity");
        Serial.println("\t(3).......Odd parity");

        while (Serial.available() < 1);
        parity = Serial.parseInt();
        if (parity == 0) {
          Serial.println("Canceled");
        } else {
          switch(parity) {
            case 1:
              command = "AT+PN" + lineEnding;
              Serial.println("Setting to No Parity check");
              Serial.println("sending command:" + command);
              Serial1.print(command);
              Serial1.flush();
              Serial1.end();
              Serial1.begin(baudRateList[baudRate], parityList[parity]);
              break;
            case 2:
              command = "AT+PE" + lineEnding;
              Serial.println("Setting to Even Parity check");
              Serial.println("sending command:" + command);
              Serial1.print(command);
              Serial1.flush();
              Serial1.end();
              Serial1.begin(baudRateList[baudRate], parityList[parity]);
              break;
            case 3:
              command = "AT+PO" + lineEnding;
              Serial.println("Setting to Odd Parity check");
              Serial.println("sending command:" + command);
              Serial1.print(command);
              Serial1.flush();
              Serial1.end();
              Serial1.begin(baudRateList[baudRate], parityList[parity]);
              break;
            default:
              command = "AT" + lineEnding;     // to avoid error in sending command below
              Serial.println("Invalid entry");
          }
          if ((command.length() - lineEnding.length()) > 2) {
            Serial.println("To complete change of parity, remove then reconnect power to HC-06.");
            Serial.println("Enter any character when complete (LED should be blinking).");
            while (Serial.available() < 1);
            Serial.readString();   // clear buffer
//            delay(1000); 
            while (Serial1.available() > 0) {
              Serial1.readString();   // clear buffer
              delay(100); 
            }
            // repeat command since response may be missed while changing baud rate
            Serial.println("Verifying completion - sending command:" + command);
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
        break;
      default:
        Serial.println("Invalid entry");
        break;
    }
    Serial.println();
    Serial.println("Select option:");
    Serial.println("\t(1) Set local Baud Rate (initially 9600)");
    Serial.println("\t(2) Set local parity (initially none)");
    Serial.println("\t(3) Get version (useful to verify connection/baud)");
    Serial.println("\t(4) Set HC06 Baud Rate");
    Serial.println("\t(5) Set HC06 BT name");
    Serial.println("\t(6) Set HC06 BT password");
    Serial.println("\t(7) Set HC06 parity");
  }
  
  delay(100);
}
