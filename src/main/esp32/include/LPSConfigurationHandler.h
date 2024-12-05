#ifndef LPS_CONFIGURATION_LED_HANDLER_H
#define LPS_CONFIGURATION_LED_HANDLER_H

class LPSConfigurationHandler
{
public:
    static void init();
    static void set_config_mode(bool value);
    static bool config_mode();
};

#endif
