#ifndef LPSDEVICE_H
#define LPSDEVICE_H

#include <stdint.h>

extern const char *LPS_DEVICE_MANUFACTURER_PREFIX; // 'LP' are part of the company id while 'S' is the first byte in the data body

typedef struct
{
    uint16_t id;
    uint16_t rssi;
} LPSDEVICE;

uint8_t serializeDevice(LPSDEVICE device); // serialized an LPSDEVICE according to the LPS protocol

LPSDEVICE *deserializeDevice(uint8_t *buffer); // deserialized an LPSDEVICE according to the LPS protocol

#endif