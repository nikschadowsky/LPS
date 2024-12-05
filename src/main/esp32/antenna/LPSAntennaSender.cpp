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
void handle_illegal_action()
{
    server.send(403, "text/html", ILLEGAL_ACTION_HTML.c_str());
}

// handles an illegal action when currently in config mode.
void handle_IllegalAction_ConfigMode()
{
    server.send(403, "text/html", ILLEGAL_ACTION_CONFIG_MODE_HTML.c_str());
}

// handle a request to /api/scan
void handle_request()
{
    if (!LPSConfigurationHandler::config_mode())
    {
        std::vector<LPSDEVICE> devices = LPSSCANNER::scan();

        // buffer size is one larger than the device count since the protocol defines a 0x00 00 00 ending.
        uint16_t size = (devices.size() + 1) * SERIALIZED_DEVICE_SIZE;

        int8_t *serialized_data_buffer = new int8_t[size];
        serialize_devices(devices.data(), serialized_data_buffer, devices.size());

        // end on 0x00 00 00
        serialized_data_buffer[size - 3] = 0;
        serialized_data_buffer[size - 2] = 0;
        serialized_data_buffer[size - 1] = 0;

        server.send_P(200, "application/octet-stream", (char *)serialized_data_buffer, size);

        delete[] serialized_data_buffer;
    }
    else
    {
        handle_IllegalAction_ConfigMode();
    }
}

// handles a POST request to /api/config. Toggles the config mode of this antenna.
void handle_config_enable()
{
    LPSConfigurationHandler::set_config_mode(1);

    server.send(202, "text/html", SUCCESSFUL_CONFIG_TOGGLE_HTML.c_str());
}

void handle_config_disable()
{
    LPSConfigurationHandler::set_config_mode(0);

    server.send(202, "text/html", SUCCESSFUL_CONFIG_TOGGLE_HTML.c_str());
}

// handles a GET request to /api/config. Returns a basic HTML view of the current status.
void handle_get_config_mode()
{
    std::string message = "<h1>Config mode is: ";
    message.append(std::to_string(LPSConfigurationHandler::config_mode()));
    message.append("</h1>");
    server.send(202, "text/html", message.c_str());
}

// Configures the WiFi and starts the webserver on port 80.
void LPSANTENNASENDER::init_server()
{
    LPSSCANNER::init("");
    LPSConfigurationHandler::init();

    WiFi.begin(LPS_SSID.c_str(), LPS_PASSCODE.c_str());

    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        Serial.println(".");
    }

    Serial.println("Connected!");
    Serial.print("IP-Address: ");
    Serial.println(WiFi.localIP());

    server.on("/api/scan", HTTP_GET, handle_request);
    server.on("/api/config/enable", HTTP_POST, handle_config_enable);
    server.on("/api/config/disable", HTTP_POST, handle_config_disable);
    server.on("/api/config", HTTP_GET, handle_get_config_mode);
    server.onNotFound(handle_illegal_action);

    server.begin();
}

// Handles clients.
void LPSANTENNASENDER::handle_requests()
{
    server.handleClient();
}
