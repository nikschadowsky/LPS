#include "LPSPositionEstimator.h"
#include <stdint.h>
#include <math.h>
#include <HardwareSerial.h>

// reference RSSI at a distance of 1m from an antenna
const int16_t REFERENCE_RSSI = -50;

// worst case scenario. has to be dialed in by testing.
const float ENVIRONMENTAL_PATH_LOSS = 4.0f;

const float THRESHOLD_ACCURACY = 0.0f;

const uint16_t MAX_ITERATIONS_GRAD_DESCENT = 1000;
const float EPSILON_GRAD_DESCENT = 1e-6;
const float LEARNING_RATE_GRAD_DESCENT = 0.01f;

float boundary_distance(const Point *point, const Antenna *antenna, const float distance)
{
    return hypotf(point->x - antenna->position.x, point->y - antenna->position.y) - distance;
}

float objective_function(const Point *predicted_center, const Antenna antenna[3], const float distance[3])
{
    float d1 = boundary_distance(predicted_center, &antenna[0], distance[0]);
    float d2 = boundary_distance(predicted_center, &antenna[1], distance[1]);
    float d3 = boundary_distance(predicted_center, &antenna[2], distance[2]);

    return (d1 - d2) * (d1 - d2) + (d1 - d3) * (d1 - d3);
}

LPSPosition gradient_descent(const uint16_t id, const Antenna antenna[3], const float distance[3])
{
    Point approximation = {
        (antenna[0].position.x + antenna[1].position.x + antenna[2].position.x) / 3.0f,
        (antenna[0].position.y + antenna[1].position.y + antenna[2].position.y) / 3.0f};

    float f, gradX, gradY;
    Point nextX;
    Point nextY;

    for (int16_t i = 0; i < MAX_ITERATIONS_GRAD_DESCENT; i++)
    {
        f = objective_function(&approximation, antenna, distance);

        nextX = {approximation.x + EPSILON_GRAD_DESCENT, approximation.y};
        nextY = {approximation.x, approximation.y + EPSILON_GRAD_DESCENT};

        gradX = (objective_function(&nextX, antenna, distance) - f) / EPSILON_GRAD_DESCENT;
        gradY = (objective_function(&nextY, antenna, distance) - f) / EPSILON_GRAD_DESCENT;

        approximation.x -= LEARNING_RATE_GRAD_DESCENT * gradX;
        approximation.y -= LEARNING_RATE_GRAD_DESCENT * gradY;
    }

    float uncertainty = fabsf(boundary_distance(&approximation, &antenna[0], distance[0]));

    return LPSPosition{id, approximation, uncertainty};
}

bool lps_position_is_valid(LPSPosition *position)
{
    return infinityf() == position->uncertainty;
}

float estimate_distance(const LPSDEVICE *device_ptr)
{
    if (device_ptr)
    {
        float exponent = (REFERENCE_RSSI - device_ptr->rssi) / (ENVIRONMENTAL_PATH_LOSS * 10.0f);
        return pow10f(exponent);
    }

    return infinityf();
}

Point calculate_midpoint(Point *a, Point *b)
{
    return Point{(a->x + b->x) / 2.0f, (a->y + b->y) / 2.0f};
}

/**
 * Null ptr imply no measurement data
 */
LPSPosition estimate_position(
    const uint16_t id,
    const LPSRoom *room_ptr,
    const LPSDEVICE *measurementA_ptr,
    const LPSDEVICE *measurementB_ptr,
    const LPSDEVICE *measurementC_ptr,
    const LPSDEVICE *measurementD_ptr)
{
    // return LPSPosition{ab_distance(room_ptr), ad_distance(room_ptr)};

    float distances[4] = {estimate_distance(measurementA_ptr),
                          estimate_distance(measurementB_ptr),
                          estimate_distance(measurementC_ptr),
                          estimate_distance(measurementD_ptr)};

    LPSPosition positions[4];
    Antenna antenna_buf[3];
    float distance_buf[3];

    // valid cases: 4 points (all antennae sent data), 1 point (one antenna did not sent data) or no points (at least two antennae did not sent data)
    uint8_t bitmap = 0;

    // #1 ABC
    if (measurementA_ptr && measurementB_ptr && measurementC_ptr)
    {
        antenna_buf[0] = room_ptr->corner[0];
        antenna_buf[1] = room_ptr->corner[1];
        antenna_buf[2] = room_ptr->corner[2];

        distance_buf[0] = distances[0];
        distance_buf[1] = distances[1];
        distance_buf[2] = distances[2];

        positions[0] = gradient_descent(id, antenna_buf, distance_buf);
        bitmap |= 1;
    }

    // #2 BCD
    if (measurementB_ptr && measurementC_ptr && measurementD_ptr)
    {
        antenna_buf[0] = room_ptr->corner[1];
        antenna_buf[1] = room_ptr->corner[2];
        antenna_buf[2] = room_ptr->corner[3];

        distance_buf[0] = distances[1];
        distance_buf[1] = distances[2];
        distance_buf[2] = distances[3];

        positions[1] = gradient_descent(id, antenna_buf, distance_buf);
        bitmap |= 2;
    }

    // #3 CDA
    if (measurementC_ptr && measurementD_ptr && measurementA_ptr)
    {
        antenna_buf[0] = room_ptr->corner[2];
        antenna_buf[1] = room_ptr->corner[3];
        antenna_buf[2] = room_ptr->corner[0];

        distance_buf[0] = distances[2];
        distance_buf[1] = distances[3];
        distance_buf[2] = distances[0];

        positions[2] = gradient_descent(id, antenna_buf, distance_buf);
        bitmap |= 4;
    }

    // #4 DAB
    if (measurementD_ptr && measurementA_ptr && measurementB_ptr)
    {
        antenna_buf[0] = room_ptr->corner[3];
        antenna_buf[1] = room_ptr->corner[0];
        antenna_buf[2] = room_ptr->corner[1];

        distance_buf[0] = distances[3];
        distance_buf[1] = distances[0];
        distance_buf[2] = distances[1];

        positions[3] = gradient_descent(id, antenna_buf, distance_buf);
        bitmap |= 8;
    }

    if (bitmap == 15)
    {
        // we have four point data

        Point midpoint_02 = calculate_midpoint(&positions[0].position, &positions[2].position);
        Point midpoint_13 = calculate_midpoint(&positions[1].position, &positions[3].position);

        Point midpoint = calculate_midpoint(&midpoint_02, &midpoint_13);
        float uncertainty = fmaxf(fmaxf(fmaxf(positions[0].uncertainty, positions[1].uncertainty), positions[2].uncertainty), positions[3].uncertainty);

        return LPSPosition{id, midpoint, uncertainty};
    }

    if (bitmap)
    {
        // we have one point of data
        return positions[bitmap];
    }

    // no point could be calculated
    return {id, {infinityf(), infinityf()}, infinityf()};
}

void serialize_lps_positions(const LPSPosition *position_ptr, uint8_t *target_buffer, uint16_t position_count)
{
    for (uint16_t i = 0; i < position_count; i++)
    {
        uint16_t offset = i * SERIALIZED_POSITION_SIZE;

        // Serialize id in big-endian
        target_buffer[offset] = (position_ptr[i].id >> 8) & 0xFF;
        target_buffer[offset + 1] = position_ptr[i].id & 0xFF;

        // Serialize position.x in big-endian
        uint8_t *x_ptr = (uint8_t *)&position_ptr[i].position.x;
        target_buffer[offset + 2] = x_ptr[3];
        target_buffer[offset + 3] = x_ptr[2];
        target_buffer[offset + 4] = x_ptr[1];
        target_buffer[offset + 5] = x_ptr[0];

        // Serialize position.y in big-endian
        uint8_t *y_ptr = (uint8_t *)&position_ptr[i].position.y;
        target_buffer[offset + 6] = y_ptr[3];
        target_buffer[offset + 7] = y_ptr[2];
        target_buffer[offset + 8] = y_ptr[1];
        target_buffer[offset + 9] = y_ptr[0];

        // Serialize uncertainty in big-endian
        uint8_t *uncertainty_ptr = (uint8_t *)&position_ptr[i].uncertainty;
        target_buffer[offset + 10] = uncertainty_ptr[3];
        target_buffer[offset + 11] = uncertainty_ptr[2];
        target_buffer[offset + 12] = uncertainty_ptr[1];
        target_buffer[offset + 13] = uncertainty_ptr[0];
    }
}
