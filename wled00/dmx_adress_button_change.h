#pragma once

#include "wled.h"
#include "fcn_declare.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// const uint32_t CLK = 18;
// const uint32_t DIO = 5;
const uint32_t BTN_UP = 19;
const uint32_t BTN_DOWN = 32;
const uint32_t BTN_SAVE = 17;
const uint32_t BTN_CH_MODE = 33;
const uint32_t REPEAT_TIME = 20;
const uint32_t DMX_MAX = 512;
const uint32_t SENSOR = 14;

enum Mode {
  SELECT_MODE = 0,
  DMX_ADRESS,
  DMX_MODE,
  PRESET,
  NUM_MODES // has to be last variant
};

String modeString(int mode) {
  switch(mode) {
    case SELECT_MODE:
      return String("Select mode");
    case DMX_ADRESS:
      return String("DMX Adress");
    case DMX_MODE:
      return String("DMX Mode");
    case PRESET:
      return String("Preset");
    return String("");
  }
  return String("");
}

enum DmxMode {
  SINGLE_RGB = 0,
  MULTIPLE_RGBW,
  NUM_DMX_MODES // has to be last variant
};

String dmxModeString(int mode) {
  switch (mode) {
    case SINGLE_RGB:
      return String("Single RGB");
    case MULTIPLE_RGBW:
      return String("Multiple\r\nRGBW");
  }
}

int curstate_btn_up = HIGH;
int laststate_btn_up = HIGH;
int curstate_btn_down = HIGH;
int laststate_btn_down = HIGH;
int curstate_btn_save = HIGH;
int laststate_btn_save = HIGH;
int curstate_btn_mode = HIGH;
int laststate_btn_mode = HIGH;

int repeat_count_up = 0;
int repeat_count_down = 0;

int mode = SELECT_MODE;
int selected = 1;
int save_msg = 0;

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

OneWire oneWire(SENSOR_PIN);
DallasTemperature DS18B20(&oneWire);

class DmxAdressButtonChange : public Usermod {
private:

public:

  DmxAdressButtonChange(const char *name, bool enabled): Usermod(name, enabled) {}

  void setup() {
    Serial.begin(115200);
    DS18B20.begin();

    if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
      Serial.println("failed to start SSD1306 OLED");
      while (1);
    }

    delay(2000);
    oled.clearDisplay();
    oled.setTextColor(WHITE);

    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_SAVE, INPUT_PULLUP);
    pinMode(BTN_CH_MODE, INPUT_PULLUP);
  }

  void loop() {
    curstate_btn_up = digitalRead(BTN_UP);
    curstate_btn_down = digitalRead(BTN_DOWN);
    curstate_btn_save = digitalRead(BTN_SAVE);
    curstate_btn_mode = digitalRead(BTN_CH_MODE);
    DS18B20.requestTemperatures();

    double temp = DS18B20.getTempCByIndex(0);

    oled.clearDisplay();

    if (curstate_btn_up == LOW && laststate_btn_up == HIGH) {
      btnUpPressed();
    }
    if (curstate_btn_up == LOW) {
      if (repeat_count_up > REPEAT_TIME) {
        btnUpPressed();
      }
      repeat_count_up += 1;
    } else {
      repeat_count_up = 0;
    }

    if (curstate_btn_down == LOW && laststate_btn_down == HIGH) {
      btnDownPressed();
    }
    if (curstate_btn_down == LOW) {
      if (repeat_count_down > REPEAT_TIME) {
        btnDownPressed();
      }
      repeat_count_down += 1;
    } else {
      repeat_count_down = 0;
    }

    if (curstate_btn_save == LOW && laststate_btn_save == HIGH) {
      btnSavePressed();
    }
    if (curstate_btn_mode == LOW && laststate_btn_mode == HIGH) {
      btnModePressed();
    }

    drawScreen();

    if (save_msg > 0) {
      oled.setTextSize(1);
      oled.setCursor(64, 45);
      oled.println("saved");
      save_msg -= 1;
    }

    oled.setTextSize(1);
    oled.setCursor(64, 0);
    oled.print((int)temp);
    oled.println("C");

    oled.display();

    laststate_btn_up = curstate_btn_up;
    laststate_btn_down = curstate_btn_down;
    laststate_btn_save = curstate_btn_save;
    laststate_btn_mode = curstate_btn_mode;
  }

  void btnUpPressed() {
    switch(mode) {
      case SELECT_MODE:
        selected -= 1;
        if (selected <= 0) {
          selected = NUM_MODES - 1;
        }
        break;
      case DMX_ADRESS:
        incDmxAddress();
        break;
      case DMX_MODE:
        selected = (selected + NUM_DMX_MODES - 1) % NUM_DMX_MODES;
        break;
      case PRESET:
        if (selected > 1) {
          selected -= 1;
        } else {
          String s = String();
          do {
            selected += 1;
          } while (getPresetName(selected, s));
          selected -= 1;
        }
        break;
    }
  }

  void btnDownPressed() {
    switch (mode) {
      case SELECT_MODE:
        selected += 1;
        if (selected >= NUM_MODES) {
          selected = 1;
        }
        break;
      case DMX_ADRESS:
        decDmxAddress();
        break;
      case DMX_MODE:
        selected = (selected + 1) % NUM_DMX_MODES;
        break;
      case PRESET:
        selected += 1;
        String s = String();
        if (!getPresetName(selected, s)) {
          selected = 1;
        }
    }
  }

  void btnSavePressed() {
    switch (mode) {
      case SELECT_MODE:
        mode = selected;
        switch (mode) {
          case DMX_MODE:
            selected = SINGLE_RGB;
            break;
          case PRESET:
            selected = 1;
            break;
        }
        break;
      case DMX_ADRESS:
        saveDmxAddress();
        save_msg = 100;
        break;
      case DMX_MODE:
        switch (selected) {
          case SINGLE_RGB:
            DMXMode = DMX_MODE_SINGLE_RGB;
            break;
          case MULTIPLE_RGBW:
            DMXMode = DMX_MODE_MULTIPLE_RGBW;
            break;
        }
        mode = DMX_ADRESS;
        save_msg = 100;
        break;
      case PRESET:
        applyPreset(selected);
        handlePresets();
        save_msg = 100;
        break;
    }
  }

  void btnModePressed() {
    mode = SELECT_MODE;
    selected = 1;
  }

  void drawScreen() {
    String header = modeString(mode);
    String s;
    switch(mode) {
      case SELECT_MODE:
        s = modeString(selected);
        drawOptionScreen(header, s);
        break;
      case DMX_ADRESS:
        s = String("") + DMXAddress;
        drawOptionScreenBigCentered(header, s);
        break;
      case DMX_MODE:
        s = dmxModeString(selected);
        drawOptionScreen(header, s);
        break;
      case PRESET:
        s = String();
        getPresetName(selected, s);
        drawOptionScreen(header, s);
        break;
    }
  }

  void drawOptionScreen(String header, String option) {
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.println(header);

    oled.setTextSize(2);
    oled.setCursor(2, 20);
    oled.println(option);
  }

  void drawOptionScreenBigCentered(String header, String option) {
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.println(header);

    oled.setTextSize(3);
    oled.setCursor(30, 20);
    oled.println(option);
  }

  void saveDmxAddress() {
      serializeConfig();
      Serial.println("DMX-Address is saved: " + DMXAddress);
  }

  void incDmxAddress() {
    DMXAddress = (DMXAddress % DMX_MAX) + 1;
    Serial.println("DMX-Address is set to: " + DMXAddress);
  }

  void decDmxAddress() {
    DMXAddress = (DMXAddress + DMX_MAX - 2) % DMX_MAX + 1;
    Serial.println("DMX-Address is set to: " + DMXAddress);
  }
};
