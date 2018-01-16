/**
 * Bluetooth-Eddystone-Beacon
 *
 * Basis kommt von hier:
 *      https://github.com/pcbreflux/espressif/blob/master/esp32/app/ble_app_ibeacon/main/app_bt.c
 *
 * YT-Erklärung:
 *      https://www.youtube.com/watch?v=TqAWQNyWHqc
 *
 * arduino-BLEPeripheral
 *      https://github.com/sandeepmistry/arduino-BLEPeripheral/blob/master/src/EddystoneBeacon.cpp
 *
 * ESP-IDF Programming Guide:
 *      http://esp-idf.readthedocs.io/en/latest/index.html
 *
 * Next: https://github.com/nkolban/ESP32_BLE_Arduino/blob/master/examples/BLE_scan/BLE_scan.ino
 */

#include "stdafx.h"
#include "config.h"
#include "ota.h"

// Internal network I (192.168.0.0/24)
const std::string ssid{ "MikeMitterer-LS" };
const std::string password{ Eddystone_PASSWORD_LS };

void setup() {
    Serial.begin(115200);
    while(!Serial && !Serial.available()){}

    Log.begin(LOG_LEVEL_NOTICE, &Serial);
    //Log.setPrefix(printTimestamp); // Uncomment to get timestamps as prefix
    //Log.setSuffix(printNewline); // Uncomment to get newline as suffix

    Log.notice(F("Booting..." CR));
    if(initBLE() == 1) {
        return;
    }

    //pinMode(internalLED, OUTPUT);
    runWebServer(ssid,password);

    initOTA();
    ArduinoOTA.begin();

    xTaskCreate(
            bleAdvtTask,      /* Task function. */
            "bleAdvtTask",    /* String with name of task. */
            2048,             /* Stack size in words. */
            NULL,             /* Parameter passed as input of the task */
            5,                /* Priority of the task. */
            NULL);            /* Task handle. */
}

void loop() {
    ArduinoOTA.handle();
}

//void printTimestamp(Print* _logOutput) {
//    char c[12];
//    int m = sprintf(c, "%10lu ", millis());
//    _logOutput->print(c);
//}
//
//void printNewline(Print* _logOutput) {
//    _logOutput->print('\n');
//}