#ifndef LPSSECRET_H
#define LPSSECRET_H

#include <string>

/**
 *  Defines the SSID of the WiFi network the antennas connect to. An associated CPP file has to be created and filled manually!
 */
extern const std::string LPS_SSID;

/**
 * Defines the passcode of the WiFi network the antennas connect to. An associated CPP file has to be created and filled manually!
 */
extern const std::string LPS_PASSCODE;

/**
 * Defines the port on which all antennas listen for requests.
 */
extern const uint16_t LPS_ANTENNA_PORT;

#endif