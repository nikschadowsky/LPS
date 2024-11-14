#include "esp_wifi.h"
#include <HTTPClient.h>
#include "LPSSecret.h"
#include <vector>

const int8_t REQUEST_TIMEOUT_MILLIS = 1500;

void setupWiFi()
{
    WiFi.mode(WIFI_AP);
    Serial.print("Setting up Access Point...");
    WiFi.softAP(LPS_SSID.c_str(), LPS_PASSCODE.c_str());

    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
}

void sendGETRequest(IPAddress antennaIP)
{
    HTTPClient http;
    String url = "http://" + antennaIP.toString() + ":" + LPS_ANTENNA_PORT;

    Serial.print("Sending GET request to: ");
    Serial.println(url);

    http.begin(url);

    int httpResponseCode = http.GET();

    if (httpResponseCode == 200)
    {
        Serial.println("Response received:");
        int len = http.getSize();

        // create array on stack
        int8_t payload[len];
        // copy body to array
        http.getStream().readBytes((char *)payload, len);

        // TODO use response
    }
    else
    {
        Serial.print("GET request failed with code: ");
        Serial.println(httpResponseCode);
    }
    http.end();
}

void setup()
{
    Serial.begin(115200);
    setupWiFi();
}

void loop()
{
    int numClients = WiFi.softAPgetStationNum();
    if (numClients > 0)
    {
        Serial.print("Number of connected devices: ");
        Serial.println(numClients);

        wifi_sta_list_t stationList;
        esp_netif_sta_list_t netif_sta_list;

        esp_wifi_ap_get_sta_list(&stationList);
        esp_netif_get_sta_list(&stationList, &netif_sta_list);

        for (int i = 0; i < numClients; i++)
        {

            esp_ip4_addr_t ip4 = netif_sta_list.sta[i].ip;
            IPAddress clientIP(ip4.addr);

            Serial.print("Device ");
            Serial.print(i + 1);
            Serial.print(" IP address: ");
            Serial.println(clientIP);

            xTaskCreate(sendGETRequest, );

            sendGETRequest(clientIP);
        }
    }

    delay(1000);
}
