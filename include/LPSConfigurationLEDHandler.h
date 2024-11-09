#ifndef LPS_CONFIGURATION_LED_HANDLER_H
#define LPS_CONFIGURATION_LED_HANDLER_H

class LPSConfigurationLEDHandler {
public:
    static void init();
    static void toggleConfigMode();
    static bool configMode();
};

#endif
