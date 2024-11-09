#include <HardwareSerial.h>
#include <WiFi.h>
#include <WebServer.h>
#include <algorithm>

#include "LPSAntennaSender.h"
#include "LPSScanner.h"
#include "LPSSecret.h"

const std::string LPSANTENNASENDER::LPS_SERVICE_DEVICE_NAME = "";

const std::string NOT_FOUND_HTML = "<h1>Invalid!</h1>";

// open a server on port 80
WebServer server(80);

void handle_Request()
{
    std::vector<LPSDEVICE> devices = LPSSCANNER::scan();

    // buffer size is one larger than the device count since the protocol defines a 0x00 00 00 ending.
    uint16_t size = (devices.size() + 1) * SERIALIZED_DEVICE_SIZE;
    int8_t buffer[size];
    for (int i = 0; i < devices.size(); i++)
    {
        LPSDEVICE device = devices.at(i);
        std::array<int8_t, SERIALIZED_DEVICE_SIZE> data = serializeDevice(device);
        std::copy(data.begin(), data.end(), buffer + i);
    }

    // end on 0x00 00 00
    buffer[size - 3] = 0;
    buffer[size - 2] = 0;
    buffer[size - 1] = 0;
    
    server.send_P(200, "application/octet-stream", (char*)buffer, size);
}

void handle_NotFound()
{
    server.send(404, "text/html", NOT_FOUND_HTML.c_str());
}

/**
 * Configures the WiFi and starts the webserver on port 80.
 */
void LPSANTENNASENDER::initServer()
{
    LPSSCANNER::init("");

    WiFi.begin(LPS_SSID.c_str(), LPS_PASSCODE.c_str());

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.println(".");
    }

    Serial.println("Connected!");
    Serial.print("IP-Address: ");
    Serial.println(WiFi.localIP());

    server.on("/", handle_Request);
    server.onNotFound(handle_NotFound);

    server.begin();
}

/**
 * Handles clients.
 */
void LPSANTENNASENDER::handleRequests()
{
    server.handleClient();
}
