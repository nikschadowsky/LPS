#include "LPSDevice.h"
#include <stddef.h>
#include <HardwareSerial.h>

std::string LPS_DEVICE_MANUFACTURER_PREFIX = "LPS"; // 'LP' are part of the company id while 'S' is the first byte in the data body

void serializeDevices(LPSDEVICE *devices_ptr, int8_t *target_buffer, uint8_t count)
{
    for (int i = 0; i < count; i++)
    {
        LPSDEVICE device = devices_ptr[i];

        target_buffer[0 + i] = (device.id >> 8) & 0xFF;
        target_buffer[1 + i] = device.id & 0xFF;
        target_buffer[2 + i] = device.rssi;
    }
}

void deserializeDevices(uint8_t *buffer, std::vector<LPSDEVICE> *target)
{
    uint8_t arraySize = 0;
    uint8_t nullCounter = 0;

    uint8_t deviceChecksum = 0;
    do
    {
        for (int i = 0; i < SERIALIZED_DEVICE_SIZE; i++)
        {
            deviceChecksum |= buffer[arraySize + i];
        }
        arraySize += 3;
    } while (deviceChecksum);

    // remove null bytes from array size
    arraySize -= 3;

    uint8_t vectorSize = arraySize % SERIALIZED_DEVICE_SIZE;

    target->reserve(vectorSize);

    for (int i = 0; i < vectorSize; i++)
    {
        uint16_t id = (buffer[i] << 8) | buffer[i + 1];
        int8_t rssi = buffer[i + 2];

        target->push_back({id, rssi});
    }
}

std::string getDeviceFormatted(LPSDEVICE &device)
{
    std::string result = "LPS-Device: " + std::to_string(device.id) + ", rssi:" + std::to_string(device.rssi);
    return result;
}