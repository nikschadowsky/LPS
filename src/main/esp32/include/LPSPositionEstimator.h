#ifndef LPS_POSITION_ESTIMATOR_H
#define LPS_POSITION_ESTIMATOR_H

#include "LPSDevice.h"
#include "LPSRoom.h"

const uint8_t SERIALIZED_POSITION_SIZE = 14; // Describes the number of bytes of a serialized position.

typedef struct
{
    uint16_t id;
    Point position;
    float uncertainty;
} LPSPosition;

bool lps_position_is_valid(LPSPosition *position);

float estimate_distance(const LPSDEVICE *device_ptr);

LPSPosition estimate_position(
    const uint16_t id,
    const LPSRoom *room_ptr,
    const LPSDEVICE *measurementA_ptr,
    const LPSDEVICE *measurementB_ptr,
    const LPSDEVICE *measurementC_ptr,
    const LPSDEVICE *measurementD_ptr);

void serialize_lps_positions(const LPSPosition *position_ptr, uint8_t *target_buffer, uint16_t position_count);

#endif