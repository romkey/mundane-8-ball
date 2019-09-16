#include "display.h"

Adafruit_SH1106 display;

bool display_setup() {
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.display();

  return true;
}

void display_handle() {
}

