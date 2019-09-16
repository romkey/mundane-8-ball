#include <Arduino.h>

#include "config.h"
#include "hw.h"

#include <EasyButton.h>

#include <multiball/wifi.h>
#include <multiball/ota_updates.h>
#include <multiball/indicator.h>
#include <multiball/mqtt.h>

#include "homebus_mqtt.h"

#include "display.h"
#include "sensors/mpu.h"

#ifdef BUILD_INFO

#define STRINGIZE_NX(A) #A
#define STRINGIZE(A) STRINGIZE_NX(A)

static const char build_info[] = STRINGIZE(BUILD_INFO);
#else
static const char build_info[] = "not set";
#endif

EasyButton button1(BUTTON_1_PIN);

void setup() {
  CRGB *leds;
  const char* hostname;
  const char *wifi_credentials[] = {
  WIFI_SSID1, WIFI_PASSWORD1,
  WIFI_SSID2, WIFI_PASSWORD2,
  WIFI_SSID3, WIFI_PASSWORD3
  };

  delay(500);

  display_setup();
  Serial.println("[display]");
  display.println("[display]");
  display.display();

  Serial.begin(115200);
  Serial.println("Hello World!");
  Serial.printf("Build %s\n", build_info);

  if(wifi_begin(wifi_credentials, 3, "mundane-8-ball")) {
    Serial.println(WiFi.localIP());
    display.println(WiFi.localIP());
    Serial.println("[wifi]");
    display.println("[wifi]");
    display.display();

    hostname = wifi_hostname();

    if(!MDNS.begin(hostname)) {
      Serial.println("Error setting up MDNS responder!");
      display.println("Error setting up MDNS responder!");
      display.display();
    } else {
      Serial.println("[mDNS]");
      display.println("[mDNS]");
      display.display();
    }

  } else {
    Serial.println("wifi failure");
    display.println("wifi failure");
    display.display();
  }

  ota_updates_setup();
  Serial.println("[ota_updates]");
  display.println("[ota_updates]");
  display.display();

  mqtt_setup(MQTT_HOST, MQTT_PORT, MQTT_UUID, MQTT_USER, MQTT_PASS);
  Serial.println("[mqtt]");
  display.println("[mqtt]");
  display.display();

  homebus_mqtt_setup();
  Serial.println("[homebus]");
  display.println("[homebus]");
  display.display();

  leds = indicator_setup(NUM_LEDS);
  FastLED.addLeds<WS2812B, LED_DATA_PIN, RGB>(leds, NUM_LEDS);
  Serial.println("[indicator]");
  display.println("[indicator]");
  display.display();

#if 0
  mpu_setup();
  Serial.println("[mpu]");
  display.println("[mpu]");
  display.display();
#endif

  button1.begin();
  button1.onPressed(homebus_button_callback);
  Serial.println("[buttons]");
  display.println("[buttons]");
  display.display();

  delay(500);
}

void loop() {
  wifi_handle();

  ota_updates_handle();

  mqtt_handle();
  //  mpu_handle();

  button1.read();

  homebus_mqtt_handle();

  display_handle();
}
