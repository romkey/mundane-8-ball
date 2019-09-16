#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

bool display_setup();
void display_handle();

extern Adafruit_SH1106 display;
