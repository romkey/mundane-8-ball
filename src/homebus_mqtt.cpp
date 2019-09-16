#include <ArduinoJson.h>

#include "homebus_mqtt.h"
#include "display.h"

#include "multiball/wifi.h"
#include "multiball/mqtt.h"

#include <Ticker.h>

// #include "multiball/uptime.h"

// static Uptime uptime;

#ifdef ESP32
extern RTC_DATA_ATTR int bootCount;
extern RTC_DATA_ATTR int wifi_failures;
#endif

extern bool status_changed;

static String homebus_endpoint;
static Ticker display_ticker;

static void homebus_mqtt_start_announcement() {};
static void homebus_mqtt_publish_status() {};

// 0 - time
// 1 - indoor temperature
// 2 - outdoor temperature
#define MAX_SCREENS 3
static uint8_t screen_number = 0;

#define DISPLAY_INTERVAL 5

static void display_ticker_callback() {
  if(++screen_number == 3)
    screen_number = 0;
}

void homebus_mqtt_setup() {
  homebus_endpoint = String("/homebus/device/") + MQTT_UUID;
  Serial.printf("homebus endpoint %s\n", homebus_endpoint.c_str());
  mqtt_subscribe((homebus_endpoint + "/cmd").c_str());

  display_ticker.attach(DISPLAY_INTERVAL, display_ticker_callback);

  mqtt_subscribe("/tick");
  mqtt_subscribe(INDOOR_WEATHER);
  mqtt_subscribe(OUTDOOR_WEATHER);

  homebus_mqtt_start_announcement();
}

void homebus_mqtt_handle() {
}

static void handle_tick(char *msg);
static void handle_indoor_values(char *msg);
static void handle_outdoor_values(char *msg);
static void update_display();

void homebus_mqtt_callback(const char* topic, char *msg) {
  unsigned length = strlen(msg);
  char* command = msg;

  Serial.printf("homebus_mqtt_callback topic: %s, msg %s\n", topic, msg);
  Serial.print("Free heap ");
  Serial.println(ESP.getFreeHeap());

  if(strcmp("/tick", topic) == 0) {
    handle_tick(msg);
    update_display();
    return;
  }

  if(strcmp(INDOOR_WEATHER, topic) == 0) {
    handle_indoor_values(msg);
    update_display();
    return;
  }

  if(strcmp(OUTDOOR_WEATHER, topic) == 0) {
    handle_outdoor_values(msg);
    update_display();
    return;
  }


  // command is meant to be a valid json string, so get rid of the quotes
  if(command[0] == '"' && command[length-1] == '"') {
    command[length-1] = '\0';
    command += 1;
  }

  Serial.printf("command %s\n", command);

  if(strcmp(command, "restart") == 0) {
    ESP.restart();
  }

  if(strncmp(command, "display clear", 13) == 0) {
    display.clearDisplay();
    display.display();
    return;
  }
}


void homebus_button_callback() {
  Serial.println("button!");

  screen_number++;
  if(screen_number == MAX_SCREENS)
    screen_number = 0;

  update_display();
}

static float last_indoor_temp = 0;
static float last_outdoor_temp = 0;

static uint8_t hour = 0, minute = 0, second = 0, month = 0, day = 0;
static uint16_t year = 0;

static void display_time() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(8,0);
  display.setTextSize(2);
  display.printf("\n%02d:%02d:%02d\n\n", hour, minute, second);
  display.printf("%02d/%02d/%04d", month, day, year);
  display.display();
}

static void display_temperature(float temp, const char* where) {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.setTextSize(2);

  display.println(where);
  display.println("temp");
  display.setTextSize(3);
  display.print(temp);
  display.display();
}

static void update_display() {
  if(screen_number == 0)
    display_time();

  if(screen_number == 1)
    display_temperature(last_indoor_temp, "indoor");

  if(screen_number == 2)
    display_temperature(last_outdoor_temp, "outdoor");
}


static void handle_tick(char *msg) {
  StaticJsonDocument<200> doc;

  Serial.printf("tick length: %d\n", strlen(msg));
  deserializeJson(doc, msg);
  Serial.printf("JSON capacity used %d\n", doc.memoryUsage());

  hour = doc["hour"];
  minute = doc["minute"];
  second = doc["second"];

  year = doc["year"];
  month = doc["month"];
  day = doc["month_day"];
}

static void handle_indoor_values(char *msg) {
  StaticJsonDocument<500> doc;

  Serial.printf("indoor length: %d\n", strlen(msg));
  deserializeJson(doc, msg);
  Serial.printf("JSON capacity used %d\n", doc.memoryUsage());

  Serial.println("temperature");
  float temp = doc["environment"]["temperature"];
  Serial.println(temp);
  last_indoor_temp = temp;
}

static void handle_outdoor_values(char *msg) {
  StaticJsonDocument<500> doc;

  Serial.printf("outdoor length: %d\n", strlen(msg));
  deserializeJson(doc, msg);
  Serial.printf("JSON capacity used %d\n", doc.memoryUsage());

  Serial.println("temperature");
  float temp = doc["weather"]["temperature"];
  Serial.println(temp);
  last_outdoor_temp = temp;
}
