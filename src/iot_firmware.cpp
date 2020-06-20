#include "Global.h"

void not_async_actions();

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("--------------started----------------");
    //Serial.setDebugOutput(true);

    setChipId();

    File_system_init();
    Serial.println("[V] LittleFS");

    CMD_init();
    Serial.println("[V] Commands");

    sensors_init();
    Serial.println("[V] Sensors");

    All_init();
    Serial.println("[V] All");

    ROUTER_Connecting();
    Serial.println("[V] ROUTER_Connecting");

    uptime_init();
    Serial.println("[V] statistics_init");

    initUpgrade();
    Serial.println("[V] initUpgrade");

    Web_server_init();
    Serial.println("[V] Web_server_init");

    web_init();
    Serial.println("[V] web_init");

    Time_Init();
    Serial.println("[V] Time_Init");

#ifdef UDP_enable
    UDP_init();
    Serial.println("[V] UDP_init");
#endif

    ts.add(
        TEST, 10000, [&](void*) {
            getMemoryLoad("[I] sysinfo ");
        },
        nullptr, true);

    just_load = false;
}

void loop() {
#ifdef OTA_enable
    ArduinoOTA.handle();
#endif

#ifdef WS_enable
    ws.cleanupClients();
#endif

    not_async_actions();

    handleMQTT();
    handleCMD_loop();
    handleButton();
    handleScenario();
#ifdef UDP_enable
    handleUdp();
#endif
    ts.update();
}

void not_async_actions() {
    do_mqtt_connection();
    do_upgrade_url();
    do_upgrade();
#ifdef UDP_enable
    do_udp_data_parse();
    do_mqtt_send_settings_to_udp();
#endif
    do_i2c_scanning();
}