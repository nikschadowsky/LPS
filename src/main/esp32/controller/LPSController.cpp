#include "esp_wifi.h"
#include <HTTPClient.h>
#include "LPSSecret.h"
#include "LPSRoom.h"
#include <vector>
#include "LPSDevice.h"
#include "LPSPositionEstimator.h"
#include "LPSRoom.h"
#include <map>
#include <array>

const uint32_t REQUEST_TIMEOUT_MILLIS = 1500;

IPAddress local_ip(192, 168, 0, 1);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

struct TCPSocketData
{
    Antenna *antenna;
    std::vector<LPSDEVICE> *device_buffer;
};

LPSRoom *room_ptr;

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

    room_ptr = new LPSRoom;

    vTaskDelay(5000/ portTICK_RATE_MS);

    Serial.write("ESP_CONFIG_START");

    // all antennas connected
    for (int i = 0; i < TOTAL_NUMBER_OF_ANTENNAS; i++)
    {
        std::string ip = get_ip_as_string(netif_sta_list.sta[i].ip.addr);

        handle_http_toggle_config_mode(ip, 0);

        // expecting A - D from visualizer
        Serial.write("ESP_CONFIG_REQ");
        while (Serial.available() < 1)
            ;

        uint8_t response = Serial.read();

        // subtracting 65 since the requested data is ASCII A - D
        // position will be set later
        // for now we just associate the ip with one of the corners
        room_ptr->corner[response - 65] = Antenna{Point{0, 0}, ip};

        handle_http_toggle_config_mode(ip, 1);
    }

    // now setting the distances...
    handle_http_toggle_config_mode(room_ptr->corner[0].ip, 0);
    handle_http_toggle_config_mode(room_ptr->corner[1].ip, 0);

    uint8_t buffer[4];
    Serial.write("ESP_CONFIG_DIST1");
    while (Serial.available() < 4)
        ;

    Serial.readBytes(buffer, 4);

    // expecting float
    float dist_ab = *(float *)buffer;
    dist_ab = 9.5f;
    handle_http_toggle_config_mode(room_ptr->corner[1].ip, 1);
    handle_http_toggle_config_mode(room_ptr->corner[3].ip, 0);

    Serial.write("ESP_CONFIG_DIST2");
    while (Serial.available() < 4)
        ;

    Serial.readBytes(buffer, 4);

    float dist_ad = *(float *)buffer;
    dist_ad = 7.80;
    room_ptr->corner[1].position.x = dist_ab;
    room_ptr->corner[2].position.x = dist_ab;
    room_ptr->corner[2].position.y = dist_ad;
    room_ptr->corner[3].position.y = dist_ad;

    handle_http_toggle_config_mode(room_ptr->corner[0].ip, 1);
    handle_http_toggle_config_mode(room_ptr->corner[3].ip, 1);
}

uint8_t HEADER_NAME_CONTENT_LENGTH_FIELD[] = "Content-Length";

uint16_t get_content_length(const uint8_t *buffer, uint16_t body_offset)
{
    uint16_t content_length_field_offset = -1;
    uint16_t field_name_length = 14;

    for (uint16_t i = 0; i < body_offset - 14; i++)
    {
        if (memcmp(buffer + i, HEADER_NAME_CONTENT_LENGTH_FIELD, 14) == 0)
        {
            content_length_field_offset = i + 15;
            break;
        }
    }

    return atoi((char *)(buffer + content_length_field_offset));
}

void handle_tcp_socket(const TCPSocketData *antenna_data[TOTAL_NUMBER_OF_ANTENNAS])
{
    uint32_t start_time;

    WiFiClient clients[TOTAL_NUMBER_OF_ANTENNAS];

    for (uint8_t i = 0; i < TOTAL_NUMBER_OF_ANTENNAS; i++)
    {
        if (clients[i].connect(antenna_data[i]->antenna->ip.c_str(), LPS_ANTENNA_PORT))
        {
            clients[i].print("GET /api/scan HTTP/1.1\r\n");
            clients[i].print("Host:");
            clients[i].print(antenna_data[i]->antenna->ip.c_str());
            clients[i].print(":");
            clients[i].print(std::to_string(LPS_ANTENNA_PORT).c_str());
            clients[i].print("\r\n");

            clients[i].print("Connection:close\r\n");
            clients[i].print("\r\n");
            clients[i].flush(); // sends request
        }
        else
        {
            // todo send prefix to serial client to inform them
            Serial.println("Antenna unreachable: " + (i + 65));
        }

        start_time = millis();
    }

    for (uint8_t i = 0; i < TOTAL_NUMBER_OF_ANTENNAS; i++)
    {
        // wait for response...
        if (clients[i].connected())
        {
            uint32_t current_time;

            while (!clients[i].available() && (current_time = millis()) - start_time <= REQUEST_TIMEOUT_MILLIS)
            {
                vTaskDelay(50 / portTICK_PERIOD_MS);
            }

            if (current_time - start_time <= REQUEST_TIMEOUT_MILLIS)
            {

                uint16_t content_size = clients[i].available();
                uint8_t *response_buffer = new uint8_t[content_size];

                clients[i].readBytes(response_buffer, content_size);

                uint16_t body_offset;

                for (uint16_t j = 0; j < content_size - 3; j++)
                {
                    if (response_buffer[j] == 0x0D && response_buffer[j + 1] == 0x0A && response_buffer[j + 2] == 0x0D && response_buffer[j + 3] == 0x0A)
                    {
                        body_offset = j + 4;
                        break;
                    }
                }

                uint16_t content_length = get_content_length(response_buffer, body_offset);

                uint8_t *body_buffer = new uint8_t[content_length];

                if (body_offset == content_size)
                {
                    // body not available yet
                    while (clients[i].available() < content_length)
                        ;
                    clients[i].readBytes(body_buffer, content_length);
                }
                else
                {
                    memcpy(body_buffer, response_buffer + body_offset, content_length);
                }

                deserialize_devices(body_buffer, antenna_data[i]->device_buffer);
                delete[] (response_buffer);
                delete[] (body_buffer);
            }
            else
            {
                // todo debug message to visualizer?
                Serial.println("Request timeout on antenna: " + i);
            }
        }

        clients[i].stop();
    }
}

const TCPSocketData *tcp_socket_data_ptr[4]; // array of pointers to the data containers; aligns with the order of the antennas in LPSRoom

const TCPSocketData *tcp_socket_data_create(Antenna *antenna)
{
    std::vector<LPSDEVICE> *antenna_devices = new std::vector<LPSDEVICE>();

    TCPSocketData *parameter = new TCPSocketData;
    parameter->antenna = antenna;
    parameter->device_buffer = antenna_devices;

    return parameter;
}

const std::map<uint16_t, std::array<LPSDEVICE, 4> *> *mapped_measurements;

struct Tuple
{
    const uint16_t id;
    LPSDEVICE *measurements;
};

std::vector<Tuple> *measurement_vector;

Tuple *get_tuple_with_id(uint16_t id)
{
    for (auto &tup : *measurement_vector)
    {
        if (tup.id == id)
        {
            return &tup;
        }
    }

    return nullptr;
}

void setup()
{
    Serial.begin(115200);
    setupWiFi();
    setupLPSRoom();

    for (uint8_t i = 0; i < TOTAL_NUMBER_OF_ANTENNAS; i++)
    {
        tcp_socket_data_ptr[i] = tcp_socket_data_create(&room_ptr->corner[i]);
    }

    measurement_vector = new std::vector<Tuple>;
}

void loop()
{
    for (auto data : tcp_socket_data_ptr)
    {
        data->device_buffer->clear();
    }

    handle_tcp_socket(tcp_socket_data_ptr);

    // create map from LPSIP and vectors and pass it to the distance estimator

    // todo iterate over each vector
    // if an id is not yet in the map, add it and create the array via MALLOC to store the device ptr at the correct index.

    for (uint8_t i = 0; i < TOTAL_NUMBER_OF_ANTENNAS; i++)
    {
        const TCPSocketData *data = tcp_socket_data_ptr[i];

        for (uint8_t j = 0; j < tcp_socket_data_ptr[i]->device_buffer->size(); j++)
        {
            LPSDEVICE device = data->device_buffer->at(j);
            uint16_t id = device.id;

            Tuple *tuple = get_tuple_with_id(id);

            if (tuple)
            {
                tuple->measurements[i] = device;
            }
            else
            {
                LPSDEVICE measurement_data[4];
                measurement_data[i] = device;

                Tuple new_tuple = Tuple{id, measurement_data};

                measurement_vector->push_back(new_tuple);
            }
        }
    }

    LPSPosition *positions = new LPSPosition[measurement_vector->size()];

    for (uint16_t i = 0; i < measurement_vector->size(); i++)
    {
        auto devices = measurement_vector->at(i).measurements;

        Serial.print(devices[0].rssi);
        Serial.print(" ");
        Serial.print(devices[1].rssi);
        Serial.print(" ");
        Serial.print(devices[2].rssi);
        Serial.print(" ");
        Serial.print(devices[3].rssi);
        Serial.println();

        positions[i] = estimate_position(measurement_vector->at(i).id, room_ptr, &devices[0], &devices[1], &devices[2], &devices[3]);

        Serial.print(positions[i].position.x);
        Serial.print(" ");
        Serial.println(positions[i].position.y);
    }

    Serial.write("ESP_POS_DATA_START");

    uint16_t buffer_size = measurement_vector->size() * SERIALIZED_POSITION_SIZE;

    uint8_t *buffer = new uint8_t[buffer_size];
    serialize_lps_positions(positions, buffer, (uint16_t)measurement_vector->size());

    Serial.write(buffer, buffer_size);
    Serial.write("POS_DATA_END");

    measurement_vector->clear();
    delete[] positions;
    delete[] buffer;
}
