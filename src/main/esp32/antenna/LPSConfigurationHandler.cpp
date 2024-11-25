#include "LPSConfigurationHandler.h"

#include <esp32-hal-gpio.h>
#include <stdint.h>
#include <string>
#include <HardwareSerial.h>

const uint8_t LED_PIN = 23; // LED Pin is GPIO34

bool _configMode;

void LPSConfigurationHandler::init()
{
    pinMode(LED_PIN, OUTPUT);
    // initialize to OFF
    digitalWrite(LED_PIN, 0);
}

void LPSConfigurationHandler::toggle_config_mode()
{
    _configMode ^= 1;
    digitalWrite(LED_PIN, _configMode);
}

bool LPSConfigurationHandler::config_mode()
{
    return _configMode;
}
