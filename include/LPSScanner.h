#ifndef LPSSCANNER_H
#define LPSSCANNER_H

#include <stdint.h>
#include <vector>
#include "LPSDevice.h"

extern const uint16_t SCAN_TIME;

void init(std::string name);

std::vector<LPSDEVICE> scan();

#endif