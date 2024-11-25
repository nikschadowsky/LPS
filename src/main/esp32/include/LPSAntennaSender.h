#ifndef LPS_ANTENNA_SENDER_H
#define LPS_ANTENNA_SENDER_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>

class LPSANTENNASENDER
{
public:
    static const std::string LPS_SERVICE_DEVICE_NAME;
    static void init_server();
    static void handle_requests();
};

#endif