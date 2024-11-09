#include <HardwareSerial.h>
#include <WiFi.h>
#include <WebServer.h>
#include <algorithm>

#include "LPSAntennaSender.h"
#include "LPSScanner.h"
#include "LPSSecret.h"
#include "LPSConfigurationHandler.h"

const std::string LPSANTENNASENDER::LPS_SERVICE_DEVICE_NAME = "";

const std::string SUCCESSFUL_CONFIG_TOGGLE_HTML = "{result: \"Operation successful!\"}";
const std::string ILLEGAL_ACTION_HTML = "<h1>Invalid Action!</h1>";
const std::string ILLEGAL_ACTION_CONFIG_MODE_HTML = "<h1>Invalid Action when in Config Mode!</h1>";

// open a server on port 80
WebServer server(80);

// handles an illegal action. an illegal action is an http request to anything other than /api/scan or /api/config.
void handle_IllegalAction()
{
    server.send(403, "text/html", ILLEGAL_ACTION_HTML.c_str());
}

// handles an illegal action when currently in config mode.
void handle_IllegalAction_ConfigMode()
{
    server.send(403, "text/html", ILLEGAL_ACTION_CONFIG_MODE_HTML.c_str());
}

// handle a request to /api/scan
void handle_Request()
{
    if (!LPSConfigurationHandler::configMode())
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

        server.send_P(200, "application/octet-stream", (char *)buffer, size);
    }
    else
    {
        handle_IllegalAction();
    }
}

// handles a POST request to /api/config. Toggles the config mode of this antenna.
void handle_ConfigRequested()
{
    Serial.println("Requested toggle of config mode...");
    LPSConfigurationHandler::toggleConfigMode();

    server.send(202, "text/html", SUCCESSFUL_CONFIG_TOGGLE_HTML.c_str());
}

// handles a GET request to /api/config. Returns a basic HTML view of the current status.
void handle_GetConfigMode()
{
    std::string message = "<h1>Config mode is: ";
    message.append(std::to_string(LPSConfigurationHandler::configMode()));
    message.append("</h1>");
    server.send(202, "text/html", message.c_str());
}

// Configures the WiFi and starts the webserver on port 80.
void LPSANTENNASENDER::initServer()
{
    LPSSCANNER::init("");
    LPSConfigurationHandler::init();

    WiFi.begin(LPS_SSID.c_str(), LPS_PASSCODE.c_str());

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.println(".");
    }

    Serial.println("Connected!");
    Serial.print("IP-Address: ");
    Serial.println(WiFi.localIP());

    server.on("/api/scan", HTTP_GET, handle_Request);
    server.on("/api/config", HTTP_POST, handle_ConfigRequested);
    server.on("/api/config", HTTP_GET, handle_GetConfigMode);
    server.onNotFound(handle_IllegalAction);

    server.begin();
}

// Handles clients.
void LPSANTENNASENDER::handleRequests()
{
    server.handleClient();
}
