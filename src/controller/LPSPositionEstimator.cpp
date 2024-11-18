#include "LPSPositionEstimator.h"
#include <stdint.h>

// reference RSSI at a distance of 1m from an antenna
const int16_t REFERENCE_RSSI = -50;

// worst case scenario. has to be dialed in by testing.
const float ENVIRONMENTAL_PATH_LOSS = 4.0;

float estimate_distance(const LPSDEVICE *device_ptr)
{
    return 0.0f;
}