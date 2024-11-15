#include "LPSDevice.h"
#include <stddef.h>
#include <HardwareSerial.h>

std::string LPS_DEVICE_MANUFACTURER_PREFIX = "LPS"; // 'LP' are part of the company id while 'S' is the first byte in the data body

std::array<int8_t, SERIALIZED_DEVICE_SIZE> serializeDevice(LPSDEVICE device)
{
    return {
        (int8_t) ((device.id >> 8) & 0xFF),
        (int8_t) (device.id & 0xFF),
        device.rssi
    };
}

std::vector<LPSDEVICE> deserializeDevices(uint8_t *buffer)
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

    std::vector<LPSDEVICE> devices = {};
    devices.reserve(vectorSize);

    for (int i = 0; i < vectorSize; i++)
    {
        uint16_t id = (buffer[i] << 8) | buffer[i + 1];
        int8_t rssi = buffer[i + 2];

        devices.push_back({id, rssi});
    }

    return devices;
}

std::string getDeviceFormatted(LPSDEVICE &device)
{
    std::string result = "LPS-Device: " + std::to_string(device.id) + ", rssi:" + std::to_string(device.rssi);
    return result;
}