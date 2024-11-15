#ifndef LPS_ROOM_H
#define LPS_ROOM_H

#include <string>

typedef struct {
    std::string IP;
} LPSIP;

typedef struct
{
    LPSIP antennaIP_A;
    LPSIP antennaIP_B;
    LPSIP antennaIP_C;
    LPSIP antennaIP_D;
    float distance_AB;
    float distance_AD;
} LPSRoom;

#endif