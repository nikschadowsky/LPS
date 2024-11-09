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
    return {};
}

std::string getDeviceFormatted(LPSDEVICE &device)
{
    std::string result = "LPS-Device: " + std::to_string(device.id) + ", rssi:" + std::to_string(device.rssi);
    return result;
}