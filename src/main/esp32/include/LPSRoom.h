#ifndef LPS_ROOM_H
#define LPS_ROOM_H

#include <string>
#include <math.h>

const uint8_t TOTAL_NUMBER_OF_ANTENNAS = 4;

typedef struct
{
    float x;
    float y;
} Point;

typedef struct
{
    Point position;
    std::string ip;
} Antenna;

/**
 *    B ----- C
 *    |       |
 *    |       |
 *    |       |
 *  Y |       |
 *  ^ |       |
 *  | A ----- D
 *  0 -> X
 */
typedef struct
{
    /**
     * [0] origin, always (0,0)
     * [1] clockwise to origin
     * [2] opposite of origin
     * [3] counterclockwise to origin
     */
    Antenna corner[TOTAL_NUMBER_OF_ANTENNAS];
} LPSRoom;

float ab_distance(const LPSRoom *room);

float ac_distance(const LPSRoom *room);

float ad_distance(const LPSRoom *room);

#endif