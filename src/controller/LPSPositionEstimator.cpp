#include "LPSPositionEstimator.h"
#include <stdint.h>
#include <math.h>

// reference RSSI at a distance of 1m from an antenna
const int16_t REFERENCE_RSSI = -50;

// worst case scenario. has to be dialed in by testing.
const float ENVIRONMENTAL_PATH_LOSS = 4.0;

const float THRESHOLD_ACCURACY = 0.0;

float estimate_distance(const LPSDEVICE *device_ptr)
{
    if (device_ptr)
    {
        float exponent = (REFERENCE_RSSI - device_ptr->rssi) / (ENVIRONMENTAL_PATH_LOSS * 10.0f);
        return pow10f(exponent);
    }

    return infinityf();
}

/**
 * Null ptr imply no measurement data
 */
LPSPosition estimate_position(
    const LPSRoom *room_ptr,
    const LPSDEVICE *measurementA_ptr,
    const LPSDEVICE *measurementB_ptr,
    const LPSDEVICE *measurementC_ptr,
    const LPSDEVICE *measurementD_ptr)
{
    float distanceA = estimate_distance(measurementA_ptr);
    float distanceB = estimate_distance(measurementB_ptr);
    float distanceC = estimate_distance(measurementC_ptr);
    float distanceD = estimate_distance(measurementD_ptr);

    LPSPosition positions[4];

    // #1 ABC
    // if all intersect
    if (measurementA_ptr && measurementB_ptr && measurementC_ptr)
    {
        if (intersects(&room_ptr->A, distanceA, &room_ptr->B, distanceB) &&
            intersects(&room_ptr->B, distanceB, &room_ptr->C, distanceC) &&
            intersects(&room_ptr->C, distanceC, &room_ptr->A, distanceA))
        {
        }
        else
        {
        }
    }

    // #2 BCD
    if (measurementB_ptr && measurementC_ptr && measurementD_ptr)
    {
        if (intersects(&room_ptr->B, distanceB, &room_ptr->C, distanceC) &&
            intersects(&room_ptr->C, distanceC, &room_ptr->D, distanceD) &&
            intersects(&room_ptr->D, distanceD, &room_ptr->B, distanceB))
        {
        }
        else
        {
        }
    }

    // #3 CDA
    if (measurementC_ptr && measurementD_ptr && measurementA_ptr)
    {
        if (intersects(&room_ptr->C, distanceC, &room_ptr->D, distanceD) &&
            intersects(&room_ptr->D, distanceD, &room_ptr->A, distanceA) &&
            intersects(&room_ptr->A, distanceA, &room_ptr->C, distanceC))
        {
        }
        else
        {
        }
    }

    // #4 DAB
    if (measurementD_ptr && measurementA_ptr && measurementB_ptr)
    {
        if (intersects(&room_ptr->D, distanceD, &room_ptr->A, distanceA) &&
            intersects(&room_ptr->A, distanceA, &room_ptr->B, distanceB) &&
            intersects(&room_ptr->B, distanceB, &room_ptr->D, distanceD))
        {
        }
        else
        {
        }
    }

    return {};
}

int8_t intersects(const vec3 *a, float radius_a, const vec3 *b, float radius_b)
{
    return hypotf(b->x - a->x, b->y - a->y) - radius_b - radius_a <= THRESHOLD_ACCURACY;
}
