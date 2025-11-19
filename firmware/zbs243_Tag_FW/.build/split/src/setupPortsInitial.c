#include "config.h"
#include "powermgt.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "asmUtil.h"
#include "board.h"
#include "cpu.h"
#include "eeprom.h"
#include "printf.h"
#include "sleep.h"
#include "timer.h"
#include "uart.h"
#include "wdt.h"


#include "function_signatures.h"

// Global variable definitions (defined only once)
uint16_t __xdata dataReqAttemptArr[POWER_SAVING_SMOOTHING] = {0};  // Holds the amount of attempts required per data_req/check-in
uint8_t __xdata dataReqAttemptArrayIndex = 0;
uint8_t __xdata dataReqLastAttempt = 0;
uint16_t __xdata nextCheckInFromAP = 0;
uint8_t __xdata wakeUpReason = 0;
uint8_t __xdata scanAttempts = 0;
int8_t __xdata temperature = 0;
uint16_t __xdata batteryVoltage = 2600;
bool __xdata lowBattery = false;
uint8_t __xdata capabilities = 0;
bool __xdata spiActive = false;
bool __xdata uartActive = false;
bool __xdata eepromActive = false;
bool __xdata i2cActive = false;

void setupPortsInitial() {
    P0INTEN = 0;
    P1INTEN = 0;
    P2INTEN = 0;
    P0FUNC = 0;
    P1FUNC = 0;
    P2FUNC = 0;
    P0DIR = 0xFF;
    P1DIR = 0xFF;
    P2DIR = 0xFF;
    P0PULL = 0x00;
    P1PULL = 0x00;
    P2PULL = 0x00;
}
