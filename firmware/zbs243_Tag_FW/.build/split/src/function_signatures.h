// Function declarations for static functions from powermgt.c
// Only includes functions that were originally static

#ifndef POWERMGT_SPLIT_H
#define POWERMGT_SPLIT_H

// Extern declarations for global variables
extern uint16_t __xdata dataReqAttemptArr[POWER_SAVING_SMOOTHING];
extern uint8_t __xdata dataReqAttemptArrayIndex;
extern uint8_t __xdata dataReqLastAttempt;
extern uint16_t __xdata nextCheckInFromAP;
extern uint8_t __xdata wakeUpReason;
extern uint8_t __xdata scanAttempts;
extern int8_t __xdata temperature;
extern uint16_t __xdata batteryVoltage;
extern bool __xdata lowBattery;
extern uint8_t __xdata capabilities;
extern bool __xdata spiActive;
extern bool __xdata uartActive;
extern bool __xdata eepromActive;
extern bool __xdata i2cActive;

// Declarations for originally static functions
void configSPI(const bool setup);

void configUART(const bool setup);

void configEEPROM(const bool setup);

void configI2C(const bool setup);

#endif // POWERMGT_SPLIT_H
