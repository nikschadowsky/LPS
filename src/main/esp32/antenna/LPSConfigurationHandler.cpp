#include "LPSConfigurationHandler.h"

#include <esp32-hal-gpio.h>
#include <stdint.h>
#include <string>
#include <HardwareSerial.h>

const uint8_t LED_PIN = 23; // LED Pin is GPIO23

bool _configMode;

void LPSConfigurationHandler::init()
{
    pinMode(LED_PIN, OUTPUT);
    // initialize to OFF
    digitalWrite(LED_PIN, 0);
}

void LPSConfigurationHandler::set_config_mode(bool value) 
{
    _configMode = value;
    digitalWrite(LED_PIN, _configMode);
}

bool LPSConfigurationHandler::config_mode()
{
    return _configMode;
}
