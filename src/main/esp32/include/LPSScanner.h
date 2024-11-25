#ifndef LPSSCANNER_H
#define LPSSCANNER_H

#include <stdint.h>
#include <vector>
#include "LPSDevice.h"

class LPSSCANNER
{
public:
    static const uint16_t SCAN_TIME;
    static void init(std::string name);
    static std::vector<LPSDEVICE> scan();
    static void deinit();
};
#endif