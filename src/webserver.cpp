//
// Created by Mike Mitterer on 16.01.18.
//

#include "stdafx.h"

static const uint8_t PORT = 80;
static const uint8_t MAX_RETRIES = 100;

AsyncWebServer server(PORT); // NOLINT

void runWebServer(std::string ssid,std::string password) {

    WiFi.begin(ssid.c_str(), password.c_str());

    Log.notice(F("Sending 'Hello World' to browser..." CR));

    digitalWrite(internalLED,LOW);
    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < MAX_RETRIES) {
        digitalWrite(internalLED,HIGH);
        delay(100);
        digitalWrite(internalLED,LOW);
    }

    if(retry >= MAX_RETRIES) {
        Log.error(F("Connection Failed! Rebooting..." CR));
        delay(5000);
        ESP.restart();
    }

    digitalWrite(internalLED,HIGH);
    Serial.println(WiFi.localIP());

    server.on("/hello", HTTP_GET, [](AsyncWebServerRequest *request){
        Log.error(F("Sending 'Hello World' to browser..." CR));
        request->send(200, "text/plain", "Hello World!!.");
    });

    server.begin();
}

