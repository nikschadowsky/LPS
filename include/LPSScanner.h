#ifndef LPSSCANNER_H
#define LPSSCANNER_H

#include <stdint.h>
#include "LPSDevice.h"

const uint16_t _SCAN_TIME = 1;

void scan();

int getLPSDeviceCount();

LPSDEVICE *getDevices();

#endif