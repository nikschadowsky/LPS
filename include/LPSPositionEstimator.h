#ifndef LPS_POSITION_ESTIMATOR_H
#define LPS_POSITION_ESTIMATOR_H

#include "LPSDevice.h"
#include "LPSRoom.h"

typedef struct
{
    float x;
    float y;
    float uncertainty;
} LPSPosition;

float estimate_distance(const LPSDEVICE *device_ptr);

LPSPosition estimate_position(
    const LPSRoom *room_ptr,
    const LPSDEVICE *measurementA_ptr,
    const LPSDEVICE *measurementB_ptr,
    const LPSDEVICE *measurementC_ptr,
    const LPSDEVICE *measurementD_ptr);

#endif