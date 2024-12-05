#include "esp_wifi.h"
#include <HTTPClient.h>
#include "LPSSecret.h"
#include "LPSRoom.h"
#include <vector>
#include "LPSDevice.h"
#include "LPSPositionEstimator.h"
#include "LPSRoom.h"
#include <map>

const uint32_t REQUEST_TIMEOUT_MILLIS = 1500;

IPAddress local_ip(192, 168, 0, 1);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

struct HttpSubTaskData
{
    Antenna *antenna;
    std::vector<LPSDEVICE> *device_buffer;
};

LPSRoom room;

void setupWiFi()
{
    WiFi.mode(WIFI_AP);
    Serial.print("Setting up Access Point...");
    WiFi.softAPConfig(local_ip, gateway, subnet);
    WiFi.softAP(LPS_SSID.c_str(), LPS_PASSCODE.c_str(), 1, 0, 5, false);
}

void handle_http_toggle_config_mode(std::string ip, bool disable)
{
    Serial.println(ip.c_str());
    std::string url = "http://";
    url.append(ip);
    url.append(":");
    url.append(std::to_string(LPS_ANTENNA_PORT));
    url.append("/api/config");
    url.append(disable ? "/disable" : "/enable");

    HTTPClient client;

    client.begin(url.c_str());
    client.POST("");
    client.end();
}

std::string get_ip_as_string(uint32_t addr)
{
    uint8_t octet4 = (addr >> 24) & 0xFF;
    uint8_t octet3 = (addr >> 16) & 0xFF;
    uint8_t octet2 = (addr >> 8) & 0xFF;
    uint8_t octet1 = addr & 0xFF;

    return std::to_string(octet1) + "." + std::to_string(octet2) + "." + std::to_string(octet3) + "." + std::to_string(octet4);
}

void setupLPSRoom()
{
    Serial.println("Waiting for all antennas to connect...");

    wifi_sta_list_t stationList;
    esp_netif_sta_list_t netif_sta_list;

    do
    {
        esp_wifi_ap_get_sta_list(&stationList);
        esp_netif_get_sta_list(&stationList, &netif_sta_list);

        Serial.println(netif_sta_list.num);

        Serial.print(".");
        vTaskDelay(500 / portTICK_PERIOD_MS);
    } while (netif_sta_list.num < TOTAL_NUMBER_OF_ANTENNAS);

    for (int i = 0; i < TOTAL_NUMBER_OF_ANTENNAS; i++)
    {
        handle_http_toggle_config_mode(get_ip_as_string(netif_sta_list.sta[i].ip.addr), 1);
    }

    Serial.write("ESP_CONFIG_START");

    // all antennas connected
    for (int i = 0; i < TOTAL_NUMBER_OF_ANTENNAS; i++)
    {
        std::string ip = get_ip_as_string(netif_sta_list.sta[i].ip.addr);

        handle_http_toggle_config_mode(ip, 0);

        // expecting A - D from visualizer
        Serial.write("ESP_CONFIG_REQ");
        while (Serial.available() < 1)
        {
        };

        uint8_t response = Serial.read();

        // subtracting 65 since the requested data is ASCII A - D
        // position will be set later
        // for now we just associate the ip with one of the corners
        room.corner[response - 65] = Antenna{Point{0, 0}, ip};

        handle_http_toggle_config_mode(ip, 1);
    }

    // now setting the distances...
    handle_http_toggle_config_mode(room.corner[0].ip, 0);
    handle_http_toggle_config_mode(room.corner[1].ip, 0);

    uint8_t buffer[4];
    Serial.write("ESP_CONFIG_DIST1");
    while (Serial.available() < 4)
    {
    };
    Serial.readBytes(buffer, 4);

    // expecting float
    float dist_ab = *(float *)buffer;
    handle_http_toggle_config_mode(room.corner[1].ip, 1);
    handle_http_toggle_config_mode(room.corner[3].ip, 0);

    Serial.write("ESP_CONFIG_DIST2");
    while (Serial.available() < 4)
    {
    };
    Serial.readBytes(buffer, 4);

    float dist_ad = *(float *)buffer;
    room.corner[1].position.x = dist_ab;
    room.corner[2].position.x = dist_ab;
    room.corner[2].position.y = dist_ad;
    room.corner[3].position.y = dist_ad;

    handle_http_toggle_config_mode(room.corner[0].ip, 1);
    handle_http_toggle_config_mode(room.corner[3].ip, 1);
}

void handle_http_scan(const HttpSubTaskData *parameter_ptr)
{
    std::string url = "http://";
    url.append(parameter_ptr->antenna->ip);
    url.append(":");
    url.append(std::to_string(LPS_ANTENNA_PORT));
    url.append("/api/scan");

    HTTPClient client;
    client.begin(url.c_str());

    uint8_t response_code = client.GET();

    if (response_code != 200)
    {
        client.end();
        return;
    }

    uint16_t payload_length = client.getSize();
    std::vector<uint8_t> payload;
    payload.reserve(payload_length);

    WiFiClient *stream = client.getStreamPtr();

    while (stream->available())
    {
        payload.push_back(stream->read());
    }

    deserialize_devices(payload.data(), parameter_ptr->device_buffer);
    client.end();
}

void handle_http_task(void *task_parameter)
{
    HttpSubTaskData *task_data_ptr = (HttpSubTaskData *)task_parameter;

    // cleanup data we dont need
    task_data_ptr->device_buffer->clear();

    handle_http_scan(task_data_ptr);

    // each task self deletes after it finishes execution
    vTaskDelete(NULL);
}

void setup()
{
    Serial.begin(115200);
    setupWiFi();
    setupLPSRoom();
}

const HttpSubTaskData *task_parameter_create(Antenna *antenna)
{
    std::vector<LPSDEVICE> *antenna_devices = new std::vector<LPSDEVICE>();

    HttpSubTaskData *parameter = new HttpSubTaskData;
    parameter->antenna = antenna;
    parameter->device_buffer = antenna_devices;

    return parameter;
}

const uint8_t NUM_SUB_TASKS = 4;

TaskHandle_t http_task_handles[NUM_SUB_TASKS];

const HttpSubTaskData *sub_task_data_ptr[NUM_SUB_TASKS] = {
    task_parameter_create(&room.corner[0]),
    task_parameter_create(&room.corner[1]),
    task_parameter_create(&room.corner[2]),
    task_parameter_create(&room.corner[3])};

const std::string task_names[NUM_SUB_TASKS] = {"antenna_A_GET", "antenna_B_GET", "antenna_C_GET", "antenna_D_GET"};

void loop()
{
    /*uint32_t timestamp = millis();

    // create tasks
    for (int i = 0; i < NUM_SUB_TASKS; i++)
    {
        xTaskCreate(handle_http_task, task_names[i].c_str(), 4096, (void *)&sub_task_data_ptr[i], 0, &http_task_handles[i]);
    }

    // since each scan task is going to take at least one second we can wait here.
    vTaskDelay(900 / portTICK_PERIOD_MS);

    while (true)
    {
        // wait out all tasks or the timeout period
        if (millis() - timestamp > REQUEST_TIMEOUT_MILLIS)
        {
            Serial.println("Some tasks did not complete in time! Terminating...");

            // if timeout period is exceeded, terminate tasks using their handle
            for (auto handle : http_task_handles)
            {
                if (eTaskGetState(handle) != eDeleted)
                {
                    vTaskDelete(handle);
                }
            }
            break;
        }

        // if all tasks terminated before the timeout we can break early
        if (eTaskGetState(http_task_handles[0]) == eDeleted &&
            eTaskGetState(http_task_handles[1]) == eDeleted &&
            eTaskGetState(http_task_handles[2]) == eDeleted &&
            eTaskGetState(http_task_handles[3]) == eDeleted)
        {
            break;
        }
        vTaskDelay(150 / portTICK_PERIOD_MS);
    }*/

    // all tasks are now completed. we need to use their vectors

    // create map from LPSIP and vectors and pass it to the distance estimator

    //std::map<uint16_t, LPSDEVICE(*)[4]> mapped_measurements;

    // todo iterate over each vector
    // if an id is not yet in the map, add it and create the array via MALLOC to store the device ptr at the correct index.

    // mapped_measurements.insert()
}
