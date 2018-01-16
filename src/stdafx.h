//
// Created by Mike Mitterer on 16.01.18.
//

#ifndef ESP32_STDAFX_H
#define ESP32_STDAFX_H

#include "ble.h"
#include "webserver.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoLog.h>

#include <bt.h>
#include <esp_log.h>
#include <esp32-hal-gpio.h>

#include <FS.h>
#include <freertos/task.h>

#ifndef ARDUINO_ARCH_ESP32
#define ARDUINO_ARCH_ESP32

#include "esp32-hal-log.h"
#endif


#endif //ESP32_STDAFX_H
