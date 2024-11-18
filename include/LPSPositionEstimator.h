#ifndef LPS_POSITION_ESTIMATOR_H
#define LPS_POSITION_ESTIMATOR_H

#include "LPSDevice.h"

typedef struct
{
    float x;
    float y;
    float uncertainty;
} LPSPosition;

float estimate_distance(const LPSDEVICE *device_ptr);

#endif