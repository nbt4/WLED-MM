#pragma once

#include "wled.h"
#include <TM1637Display.h>
#include <EEPROM.h>

const uint32_t CLK = 18;
const uint32_t DIO = 5;
const uint32_t BTN_UP = 19;
const uint32_t BTN_DOWN = 32;
const uint32_t BTN_SAVE = 17;
const uint32_t BTN_CH_MODE = 33;
const uint32_t PRESSED_TIME = 20;
const uint32_t DMX_MAX = 512;

int curstate_btn_up = HIGH;
int laststate_btn_up = HIGH;
int curstate_btn_down = HIGH;
int laststate_btn_down = HIGH;
int curstate_btn_save = HIGH;
int laststate_btn_save = HIGH;
int curstate_btn_ch_mode = HIGH;
int laststate_btn_ch_mode = HIGH;
int count_up = 0;
int count_down = 0;
int lastTime = 1000;



TM1637Display display(CLK, DIO);

const uint8_t save[] = {
  SEG_A | SEG_F | SEG_G | SEG_C | SEG_D,          // S
  SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,  // A
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,          // V
  SEG_A | SEG_F | SEG_G | SEG_E | SEG_D           // E
};

const uint8_t srgb[] = {
  SEG_F | SEG_G | SEG_B | SEG_C,                  // 4 mit Punkt (Digit 1)
  SEG_G,                                          // - (Digit 2)
  SEG_A | SEG_F | SEG_E | SEG_D,                  // C (Digit 3)
  SEG_B | SEG_C | SEG_E | SEG_F | SEG_G           // H (Digit 4)
};

const uint8_t m_rgb[] = {
  SEG_C | SEG_E | SEG_G,                          // M mit Punkt (Digit 1)
  SEG_E | SEG_G,                                  // R (Digit 2)
  SEG_A | SEG_B | SEG_G | SEG_F | SEG_C | SEG_D,  // G (Digit 3)
  SEG_C | SEG_D | SEG_E | SEG_F | SEG_G           // B (Digit 4)
};




class DmxAdressButtonChange : public Usermod {
  private:

  public:

  DmxAdressButtonChange(const char *name, bool enabled): Usermod(name, enabled) {}

    void setup() {
      
      // DMXAddress = 1;
      Serial.begin(115200);
      display.setBrightness(7);
      pinMode(BTN_UP, INPUT_PULLUP);
      pinMode(BTN_DOWN, INPUT_PULLUP);
      pinMode(BTN_SAVE, INPUT_PULLUP);
      pinMode(BTN_CH_MODE, INPUT_PULLUP);
      
      // EEPROM.begin(2554);
      // byte b0 = EEPROM.read(2551);
      // byte b1 = EEPROM.read(2552);
      // byte b2 = EEPROM.read(2553);
      // byte b3 = EEPROM.read(2554);

      // DMXAddress |= b0;
      // DMXAddress |= b1 << 8;
      // DMXAddress |= b2 << 16;
      // DMXAddress |= b3 << 24;
      
      // if(DMXAddress > DMX_MAX) {
      //   DMXAddress = 1;
      // }

    }

    void loop() {

      if(curstate_btn_save == LOW) {

        curstate_btn_save = digitalRead(BTN_SAVE);
        return;
    
      }

      if(curstate_btn_ch_mode == LOW) {

        curstate_btn_ch_mode = digitalRead(BTN_CH_MODE);
        return;
    
      }
    
      curstate_btn_up = digitalRead(BTN_UP);
      curstate_btn_down = digitalRead(BTN_DOWN);
      curstate_btn_save = digitalRead(BTN_SAVE);
      curstate_btn_ch_mode = digitalRead(BTN_CH_MODE);
      display.showNumberDec(DMXAddress);
    
      if(curstate_btn_ch_mode == LOW && laststate_btn_ch_mode == HIGH) {
        chDmxMode();
        Serial.println("DMX-Mode is set to: " + DMXMode);
      }
      
      if(curstate_btn_up == HIGH && laststate_btn_up == LOW) {
        incDmxAddress();
      }
    
      if(curstate_btn_down == HIGH && laststate_btn_down == LOW) {
        decDmxAddress();
      }
    
      if(curstate_btn_up == LOW) {
        if(count_up > PRESSED_TIME) {
    
          incDmxAddress();
    
        }
        count_up += 1;
    
      } else {
    
        count_up = 0;
    
      }
    
      if(curstate_btn_down == LOW) {
        if(count_down > PRESSED_TIME) {
    
          decDmxAddress();
    
        }
        count_down += 1;
    
      }else {
        count_down = 0;
      }
    
    
      laststate_btn_up = curstate_btn_up;
      laststate_btn_down = curstate_btn_down;
      // Serial.println(curstate);
    
      if(curstate_btn_save == LOW && laststate_btn_save == HIGH) {

        serializeConfig();

        display.setSegments(save);
        Serial.println("DMX-Address is saved: " + DMXAddress);
    
      }
    
      laststate_btn_save = curstate_btn_save;
      laststate_btn_ch_mode = curstate_btn_ch_mode;

    }

    void incDmxAddress() {
      DMXAddress = (DMXAddress % DMX_MAX) + 1;
      Serial.println("DMX-Address is set to: " + DMXAddress);
    }

    void decDmxAddress() {
      DMXAddress = (DMXAddress + DMX_MAX - 2) % DMX_MAX + 1;
      Serial.println("DMX-Address is set to: " + DMXAddress);
    }

    void chDmxMode() {
      if(DMXMode == DMX_MODE_SINGLE_RGB){
        DMXMode = DMX_MODE_MULTIPLE_RGBW;
        display.setSegments(m_rgb);
      }
      else{
        DMXMode = DMX_MODE_SINGLE_RGB;
        display.setSegments(srgb);
      }
    }

};