#ifndef LPS_CONFIGURATION_LED_HANDLER_H
#define LPS_CONFIGURATION_LED_HANDLER_H

class LPSConfigurationHandler
{
public:
    static void init();
    static void toggle_config_mode();
    static bool config_mode();
};

#endif
