#include "LPSScanner.h"
#include <BLEDevice.h>
#include <HardwareSerial.h>

const uint16_t LPSSCANNER::SCAN_TIME = 1;

void LPSSCANNER::init(std::string name)
{
    Serial.println("Initialising scanner...");
    BLEDevice::init(name);
}

// the user is responsible to clean up the resulting std::vector
std::vector<LPSDEVICE> LPSSCANNER::scan()
{
    BLEScan *scan = BLEDevice::getScan();
    scan->setActiveScan(1);
    BLEScanResults results = scan->start(SCAN_TIME);

    std::vector<LPSDEVICE> devices = {};
    devices.reserve(results.getCount());

    for (int i = 0; i < results.getCount(); i++)
    {
        BLEAdvertisedDevice advertisedDevice = results.getDevice(i);

        // skip all devices without manufacturer data
        if (!advertisedDevice.haveManufacturerData())
        {
            continue;
        }

        std::string mData = advertisedDevice.getManufacturerData();

        if (mData.rfind(LPS_DEVICE_MANUFACTURER_PREFIX, 0) == 0)
        {
            uint16_t id = ((mData.at(3) & 0xFF) << 8) | (mData.at(4) & 0xFF);
            int8_t rssi = advertisedDevice.getRSSI();

            // the mData is only a valid LPS device identification if it ends with a 0x23
            if (mData.at(5) == 0x23)
            {
                devices.push_back({id, rssi});
            }
        }
    }
    scan->clearResults();

    devices.shrink_to_fit();
    return devices;
}

void LPSSCANNER::deinit()
{
    BLEDevice::deinit();
}