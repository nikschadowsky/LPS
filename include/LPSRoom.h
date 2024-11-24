#ifndef LPS_ROOM_H
#define LPS_ROOM_H

#include <string>
#include <math.h>

struct vec3
{
    float x;
    float y;
    std::string ip;
};

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
    vec3 A; // origin, always (0, 0)
    vec3 B; // clockwise to origin
    vec3 C; // opposite of origin
    vec3 D; // counterclockwise to origin

} LPSRoom;

float ab_distance(const LPSRoom *room);

float ac_distance(const LPSRoom *room);

float ad_distance(const LPSRoom *room);

#endif