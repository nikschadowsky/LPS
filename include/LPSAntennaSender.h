#ifndef LPS_ANTENNA_SENDER_H
#define LPS_ANTENNA_SENDER_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>

class LPSANTENNASENDER
{
public:
    static const std::string LPS_SERVICE_DEVICE_NAME;
    static void initServer();
    static void handleRequests();
};

#endif