#include "LPSRoom.h"

float ab_distance(const LPSRoom *room)
{
    return room->corner[1].position.y;
}

float ac_distance(const LPSRoom *room)
{
    return hypotf(room->corner[1].position.x, room->corner[2].position.y);
}

float ad_distance(const LPSRoom *room)
{
    return room->corner[3].position.x;
}
