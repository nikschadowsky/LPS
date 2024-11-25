#include "LPSRoom.h"

float ab_distance(const LPSRoom *room)
{
    return room->B.position.y;
}

float ac_distance(const LPSRoom *room)
{
    return hypotf(room->C.position.x, room->C.position.y);
}

float ad_distance(const LPSRoom *room)
{
    return room->D.position.x;
}
