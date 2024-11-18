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
void serialize_devices(LPSDEVICE *devices_ptr, int8_t *target_buffer, uint8_t count);

void deserialize_devices(uint8_t *buffer, std::vector<LPSDEVICE> *target); // deserialized an LPSDEVICE according to the LPS protocol

std::string get_device_formatted(LPSDEVICE &device);

#endif