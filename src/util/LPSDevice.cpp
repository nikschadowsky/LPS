#include "LPSDevice.h"
#include <stddef.h>
#include <HardwareSerial.h>

std::string LPS_DEVICE_MANUFACTURER_PREFIX = "LPS"; // 'LP' are part of the company id while 'S' is the first byte in the data body

uint8_t serializeDevice(LPSDEVICE device)
{
    return 255;
}

LPSDEVICE *deserializeDevice(uint8_t *buffer)
{
    return NULL;
}

std::string getDeviceFormatted(LPSDEVICE &device)
{
    std::string result = "LPS-Device: " + std::to_string(device.id) + ", rssi:" + std::to_string(device.rssi);
    return result;
}