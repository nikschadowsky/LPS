#ifndef LPSDEVICE_H
#define LPSDEVICE_H

#include <string>
#include <array>
#include <vector>
#include <stddef.h>
#include <stdint.h>

extern std::string LPS_DEVICE_MANUFACTURER_PREFIX; // 'LP' are part of the company id while 'S' is the first byte in the data body

const uint8_t SERIALIZED_DEVICE_SIZE = 3; // Describes the number of bytes of a serialized device

typedef struct
{
    uint16_t id;
    int8_t rssi;
} LPSDEVICE;

/**
 * Serialized an LPSDEVICE according to the standard described in the documentation.
 */
std::array<int8_t, SERIALIZED_DEVICE_SIZE> serializeDevice(LPSDEVICE device);

std::vector<LPSDEVICE> deserializeDevices(uint8_t *buffer); // deserialized an LPSDEVICE according to the LPS protocol

std::string getDeviceFormatted(LPSDEVICE &device);

#endif