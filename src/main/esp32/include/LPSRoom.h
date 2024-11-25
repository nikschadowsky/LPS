#ifndef LPS_ROOM_H
#define LPS_ROOM_H

#include <string>
#include <math.h>

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
    Antenna A; // origin, always (0, 0)
    Antenna B; // clockwise to origin
    Antenna C; // opposite of origin
    Antenna D; // counterclockwise to origin

} LPSRoom;

float ab_distance(const LPSRoom *room);

float ac_distance(const LPSRoom *room);

float ad_distance(const LPSRoom *room);

#endif