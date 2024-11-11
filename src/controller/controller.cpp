#include "esp_wifi.h"
#include <HTTPClient.h>

const char *ssid = "LPS_Access_Point";
const char *password = "LPS";

void setupWiFi()
{
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t esp_wifi_init(const wifi_init_config_t &config);
    WiFi.mode(WIFI_AP);
    Serial.print("Setting up Access Point...");
    WiFi.softAP(ssid, password);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
}

void sendGETRequest(IPAddress deviceIP)
{
    HTTPClient http;
    String url = "http://" + deviceIP.toString() + ":80";

    Serial.print("Sending GET request to: ");
    Serial.println(url);

    http.begin(url);

    int httpResponseCode = http.GET();

    if (httpResponseCode == 200)
    {
        Serial.println("Response received:");
        int len = http.getSize();
        uint8_t *payload = (uint8_t *)malloc(len);

        // TODO use response

        free(payload);
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

        if (esp_wifi_ap_get_sta_list(&stationList) == ESP_OK)
        {
            if (esp_netif_get_sta_list(&stationList, &netif_sta_list) == ESP_OK)
            {
                for (int i = 0; i < numClients; i++)
                {

                    esp_ip4_addr_t ip4 = netif_sta_list.sta[i].ip;
                    IPAddress clientIP(ip4.addr);

                    Serial.print("Device ");
                    Serial.print(i + 1);
                    Serial.print(" IP address: ");
                    Serial.println(clientIP);

                    sendGETRequest(clientIP);
                }
            }
            else
            {
                Serial.println("Failed to map station list to network interface station list.");
            }
        }
        else
        {
            Serial.println("Failed to get station list.");
        }
    }

    delay(1000);
}
