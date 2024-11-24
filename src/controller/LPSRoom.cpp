#include "LPSRoom.h"

float ab_distance(const LPSRoom *room)
{
    return room->B.y;
}

float ac_distance(const LPSRoom *room)
{
    return hypotf(room->C.x, room->C.y);
}

float ad_distance(const LPSRoom *room)
{
    return room->D.x;
}
