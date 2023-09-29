/**
 * @file configureBT.h
 * 
 * HC-05/06 AT Command Center
 *  Version 2.0
 * 
 *  Description: Simple HC05/06 AT configuration program. Requires 2nd UART (Serial1) defined. 
 * 
 * 
 *  Created on: 18-Oct, 2021
 *      Author: miller4@rose-hulman.edu
 *    Modified: 27-Sep, 2023
 *    Revision: 2.0
 */

#ifndef CONFIGUREBT_H
#define CONFIGUREBT_H

#include <Arduino.h>

/** index for unknown device role */
#define ROLE_UNKNOWN         -1
/** index for HC-05 devices in secondary role */
#define ROLE_SECONDARY        0
/** index for HC-05 devices in primary role */
#define ROLE_PRIMARY          1
/** index for HC-05 devices in secondary-loop role */
#define ROLE_SECONDARY_LOOP   2


/**
 * HCBT class
 * 
 * HC05 or HC06 Bluetooth device class.  
 * Defines UART interface and methods for interaction.
 */
class HCBT
{
private:
  /**
   * initDevice
   *
   * @brief Initialize identification variables of HC-0x device.
   */
  void initDevice();

  /**
   * fetchRole
   *  
   * Send AT command to request current BT role for HC-05 device.
   * 
   * @param verboseOut     if true, prints verbose output to Serial
   * 
   * @returns  current role setting of device:
   *    - ROLE_UNKNOWN    - unknown (may be HC-06 fw version 1.x)
   *    - ROLE_SECONDARY  - acts as discoverable wireless UART device ready for transparent data exchange
   *    - ROLE_PRIMARY    - scans for a remote bluetooth (secondary) device, pairs, and setup connection
   *    - ROLE_SECONDARY_LOOP - data loop-back Rx-Tx, used mainly for testing
   */
  int fetchRole(bool verboseOut = false);

  /**
   * fetchVersion
   *  
   * Send AT command to request firmware version to Serial1 and return response.
   * 
   * @param verboseOut     if true, prints verbose output to Serial
   * 
   * @returns version string returned by HC-0x device
   */
  String fetchVersion(bool verboseOut = false);

  /**
   * selectBaudRate
   *  
   * Configure baud rate of HC-xx UART.
   * Prints menu to Serial to select desired baud rate for UART.
   */
  void selectBaudRate();

  /**
   * setBaudRate
   *  
   * Configure baud rate of HC-xx UART with firmware 1.x.
   * Sends AT command to configure baud rate of HC-xx UART and displays response.
   * If successful, updates Serial1 configuration to new UART settings.
   * 
   * @param newBaud       desired buad rate to configure HC-xx device UART
   *    - 1   - 1200
   *    - 2   - 2400
   *    - 3   - 4800
   *    - 4   - 9600
   *    - 5   - 19200
   *    - 6   - 38400
   *    - 7   - 57600
   *    - 8   - 115200
   * @param verboseOut    if true, prints verbose output to Serial
   * 
   * @returns true if setting baud rate succeeds 
   */
  bool setBaudRate(int newBaud, bool verboseOut = false);

  /**
   * changeName
   *  
   * Configure name of Bluetooth module.
   * Provides prompts to Serial to enter new BT name. Prepends 'HC05_' or 'HC06_' 
   * to user input string. 
   */
  void changeName();

  /**
   * changeRole
   *  
   * Send AT command to set BT role of HC-05 device.
   * 
   * @param role     role to set HC-05 device:
   *    - ROLE_SECONDARY  - acts as discoverable wireless UART device ready for transparent data exchange
   *    - ROLE_PRIMARY    - scans for a remote bluetooth (secondary) device, pairs, and setup connection
   *    - ROLE_SECONDARY_LOOP - data loop-back Rx-Tx, used mainly for testing
   * @param verboseOut     if true, prints verbose output to Serial
   * 
   * @returns true if request succeeds.
   */
  bool changeRole(int role, bool verboseOut);

  /**
   * changePin
   *  
   * Configure Bluetooth pin of HC-xx device.
   * Provides prompts to Serial to enter new BT pin/passkey of HC-xx device.
   */
  void changePin();

  /**
   * changeParity
   *  
   * Configure parity of HC-xx UART.
   * Prints menu to Serial to select desired parity for UART.
   */
  void changeParity();

  /**
   * setParity
   *  
   * Configure parity of HC-xx UART with firmware 1.x.
   * Sends AT command to configure parity of HC-xx UART and displays response.
   * If successful, updates Serial1 configuration to new UART settings.
   * 
   * !!!!  NOTE  !!!! 
   * Firmware version 1.x requires power-cycle of HC-06 after update to parity 
   * settings before changes become active.
   * 
   * @param parity        desired parity to configure HC-xx device UART
   *    - 0   - NOPARITY
   *    - 1   - ODDPARITY
   *    - 2   - EVENPARITY
   * @param verboseOut    if true, prints verbose output to Serial
   * 
   * @returns true if setting parity succeeds 
   */
  bool setParity(int parity, bool verboseOut = false);

  /**
   * clearStreams
   * 
   * @brief Clears Serial and Serial1 input buffers before requesting new response.
   */
  void clearStreams();

  /**
   * clearSerial
   * 
   * @brief Clears Serial input buffer before requesting new response.
   */
  void clearSerial();

  /**
   * clearInputStream
   * 
   * @brief Clears Serial1 input buffers before requesting new response.
   * 
   * @param firmware    firmware version identifier for HC-xx
   */
  void clearInputStream(int firmware);

  /**
   * responseDelay
   *  
   * @brief Delays for period necessary to allow completion of HC-xx response to command.
   * 
   * @param characters  count of characters in AT command
   * @param firmware    firmware version identifier for HC-xx
   * @param command     index of AT command (as defined in HCxxCommands)
   */
  void responseDelay(unsigned long characters, int firmware, int command);

  /**
   * testEcho
   * 
   * Send AT command to test configuration of UART.
   * If OK response not received, firmVersion set to FIRM_UNKNOWN.
   * 
   * @param verboseOut     if true, prints verbose output to Serial
   *  
   * @returns true if responds with OK
   */
  bool testEcho(bool verboseOut = false);

  /**
   * indexBaud
   * 
   * @brief Return index of baud rate value within baudRateList array.
   * 
   * @param baud          baud rate value (e.g. 57600)
   * @param verboseOut    if true, prints verbose output to Serial
   * 
   *  @returns index for baud rate or -1 if invalid baud rate
  */
  int indexBaud(unsigned long baud, bool verboseOut);

  /**
   * constructUARTstring
   *  
   * @brief Constructs string for AT command to configure UART (for firmware vers 3.x)
   * 
   * @param baud    baud rate value (e.g. 57600)
   * @param parity  parity setting
   *                - 0 - None
   *                - 1 - Odd parity
   *                - 2 - Even parity
   * @param stops   number of stop bits
   *                - 0 - 1 bit
   *                - 1 - 2 bits
   * 
   *  @returns String for AT command
   */
  String constructUARTstring(unsigned long baud, int parity, int stops);

  /**
   * printMenu
   *  
   * Print menu of options for configuration of UART or Bluetooth module. 
   * Will first check for identified connected device, and request scan if
   * firmware and configuration of connected device is not known.
   */
  void printMenu();

  // device model: HC-05 or HC-06
  int deviceModel;
  // device firmware version
  int firmVersion;
  // device role setting: secondary, primary, secondary_loop
  int deviceRole;
  // device UART baud rate setting
  int baudRate;
  // device UART parity setting
  int uartParity;
  // device UART stop bit configuration
  int stopBits;
  // device firmware version string
  String versionString;
  // Bluetooth broadcast name
  String btName;
  // UART interface for HC-0x device
  Stream *_uart;
  // pin connected to STATE output of HC-05
  int _statePin;
  // pin connected to EN/KEY input of HC-05
  int _keyPin;
  // current mode of HC-05, N/A for HC-06
  int _mode;
  // true if serial UART previously begun
  bool uartBegun;
  
public:
  /* 
   * Create instance of HCBT class.  
   * UART interface may be HardwareSerial or SoftwareSerial object,
   * Serial1 is default.
   * 
   * @param uart        serial interface for HC-0x (Serial1 is default)
   * @param keyPin      pin conencted to EN/KEY input of HC-05 (not used for HC-06)
   * @param statePin    pin conencted to STATE output of HC-05 (not used for HC-06)
   */
  // TODO add support for specifying UART for AT communication
  //  HCBT(Stream * uart = &Serial1, int keyPin = 0, int statePin = 0);

  /** 
   * @brief Default HCBT constructor.
   * 
   * Uses Serial1 for UART connection. Pin values are optional, defaults to 0.
   * 
   * @param keyPin      pin conencted to EN/KEY input of HC-05 (not used for HC-06)
   * @param statePin    pin conencted to STATE output of HC-05 (not used for HC-06)
   */
  HCBT(int keyPin = 0, int statePin = 0);

  /**
   * @brief Print user menu with config options to Serial and handle selection.
   */
  void commandMenu();

  /**
   * @brief Automated scan of Bluetooth module to determine configuration of UART.
   * 
   * Will identify version of firmware, baud rate, and parity setting, and set
   * Serial1 to match HC-xx UART settings.
   * 
   * @param verboseOut     if true, prints verbose output to Serial
   * 
   * @returns true if UART configuration successfully identified
   */
  bool detectDevice(bool verboseOut = false);

  /**
   * @brief Send AT command to request current BT role for HC-05 device.
   * 
   *  Returns cached value if role previously fetched. Cached value 
   *  reset by detectDevice().
   * 
   * @param verboseOut     if true, prints verbose output to Serial
   * 
   * @returns  current role setting of device:
   *    - ROLE_UNKNOWN    - unknown (may be HC-06 fw version 1.x)
   *    - ROLE_SECONDARY  - acts as discoverable wireless UART device ready for transparent data exchange
   *    - ROLE_PRIMARY    - scans for a remote bluetooth (secondary) device, pairs, and setup connection
   *    - ROLE_SECONDARY_LOOP - data loop-back Rx-Tx, used mainly for testing
   */
  int getRole(bool verboseOut = false);

  /**
   * @brief Send AT command to set BT role of HC-05 device.
   * 
   * @param role     role to set HC-05 device:
   *    - ROLE_SECONDARY  - acts as discoverable wireless UART device ready for transparent data exchange
   *    - ROLE_PRIMARY    - scans for a remote bluetooth (secondary) device, pairs, and setup connection
   *    - ROLE_SECONDARY_LOOP - data loop-back Rx-Tx, used mainly for testing
   * @param verboseOut     if true, prints verbose output to Serial
   * 
   * @returns true if request succeeds.
   */
  bool setRole(int role, bool verboseOut = false);

  /**
   * @brief Send AT command to request firmware version to Serial1 and return response. 
   * 
   *  Returns cached value if version string previously fetched. 
   * 
   * @param verboseOut     if true, prints verbose output to Serial
   * 
   * @returns version string returned by HC-0x device
   */
  String getVersionString(bool verboseOut = false);

  /**
   * @brief Configure baud rate and parity of HC-xx UART.
   * 
   * Sends AT command(s) to configure baud rate and parity of HC-xx UART.
   * If successful, updates Serial1 configuration to new UART settings.
   * 
   * !!!!  NOTE  !!!! 
   * Firmware version 1.x requires power-cycle of HC-06 after update to parity 
   * settings before changes become active.
   * 
   * @param baud        desired buad rate to configure HC-xx device UART (e.g. 9600)
   * @param parity      desired parity to configure HC-xx device UART
   *    - 0   - NOPARITY
   *    - 1   - ODDPARITY
   *    - 2   - EVENPARITY
   * @param verboseOut  if true, prints verbose output to Serial
   * 
   * @returns true if setting baud rate and parity succeeds 
   */
  bool configUART(unsigned long baud, int parity, bool verboseOut = false);

  /**
   * @brief Configure name of Bluetooth module.
   * 
   * Sends AT command to set Bluetooth broadcast name of HC-xx device. Some 
   * devices with firmware version 1.x exhibited failures when trying to set 
   * name to more than 14 characters with higher baud rates.
   * 
   * @param newName       desired Bluetooth name to configure HC-xx device
   * @param verboseOut    if true, prints verbose output to Serial
   * 
   * @returns true if setting name succeeds 
   */
  bool setName(String newName, bool verboseOut = false);

  /**
   * @brief Configure Bluetooth pin (passcode) of HC-xx device.
   * 
   * Sends AT command to configure BT pin/passkey of HC-xx UART device.
   * For firmware version 1.x, 4-digit code is accepted. For firmware version
   * 3.x, up to 16 alphanumeric character passkey is accepted according to 
   * documentation. This is artificially limited to 14 characters to ensure no
   * conflict with adding quotation characters.
   * 
   * @param newPin        desired Bluetooth pin/passkey for HC-xx device
   * @param verboseOut    if true, prints verbose output to Serial
   * 
   * @returns true if setting pin succeeds 
   */
  bool setPin(String newPin, bool verboseOut);

  /**
   * @brief Set EN pin high to place HC-05 in command mode.
   */
  void setCommandMode();

  /**
   * @brief Set EN pin low (or float) to place HC-05 in data mode.
   */
  void setDataMode();

  /**
   * @brief Manually configure baud rate of Serial1, for testing/debugging purposes.
   * 
   * Preferred to allow detectDevice() to automatically set configuration.
   * Prints menu to Serial to select desired baud rate.
   */
  void setLocalBaud();

  /**
   * @brief Manually configure parity of Serial1, for testing/debugging purposes.
   * 
   * Preferred to allow detectDevice() to automatically set configuration.
   * Prints menu to Serial to select desired parity setting.
   */
  void setLocalParity();

};

#endif // CONFIGUREBT_H
