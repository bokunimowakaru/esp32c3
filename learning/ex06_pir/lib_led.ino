/***********************************************************************
LED制御ドライバ RGB LED WS2812
                                   Copyright (c) 2022-2024 Wataru KUNINO
***********************************************************************/

#ifndef ESP_IDF_VERSION
    #define ESP_IDF_VERSION 0
#endif
#ifndef ESP_IDF_VERSION_VAL
    #define ESP_IDF_VERSION_VAL(major, minor, patch) ((major << 16) | (minor << 8) | (patch))
    // https://github.com/espressif/esp-idf/blob/master/components/esp_common/include/esp_idf_version.h
#endif

#define NUM_LEDS 3                              // LED数 1～3に対応

void print_esp_idf_version(){
    Serial.print(ESP_IDF_VERSION >> 16);
    Serial.print(".");
    Serial.print((ESP_IDF_VERSION >> 8) % 255);
    Serial.print(".");
    Serial.println(ESP_IDF_VERSION % 255);
}

////////////////////////////////////////////////////////////////////////
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,0,0)
////////////////////////////////////////////////////////////////////////

rmt_data_t led_data[NUM_LEDS * 24];
int _PIN_LED = 0;

void led(int r,int g,int b){                    // LEDにカラーを設定
    if(_PIN_LED == 0) return;
    uint32_t rgb = (g & 0xff) << 16 | (r & 0xff) << 8 | (b & 0xff);
    for (int bit = 0; bit < NUM_LEDS * 24; bit++) {
        if ((rgb & (1 << (23 - (bit % 24)))) ) {
            led_data[bit].level0 = 1;
            led_data[bit].duration0 = 8;
            led_data[bit].level1 = 0;
            led_data[bit].duration1 = 4;
        } else {
            led_data[bit].level0 = 1;
            led_data[bit].duration0 = 4;
            led_data[bit].level1 = 0;
            led_data[bit].duration1 = 8;
        }
    }
    rmtWrite(_PIN_LED, led_data, NUM_LEDS * 24, RMT_WAIT_FOR_EVER);
}

void led(int brightness){                       // グレースケール制御
    if(brightness > 0xff) brightness = 0xff;    // 256以上時に255に設定
    led(brightness,brightness,brightness);      // RGB全て同値でLED制御
}

void led_on(){                                  // LED制御の停止
    led(30);                                    // LEDの消灯
}

void led_off(){                                 // LED制御の停止
    led(0);                                     // LEDの消灯
}

void led_setup(int pin){
    _PIN_LED = pin;
    if(_PIN_LED == 0) return;
    Serial.println("RMT Init, Espressif Systems Remote Control Transceiver, forked by Wataru KUNINO");
    Serial.print("RMT Init, ESP_IDF_VERSION: ");
    print_esp_idf_version();
    Serial.println("RMT Init, (PIN="+String(pin)+") real tick set to: 100ns");
    rmtInit(_PIN_LED, RMT_TX_MODE, RMT_MEM_NUM_BLOCKS_1, 10000000);
}

/***********************************************************************
参考文献 RMT Write RGB LED
Remote Control Transceiver (RMT) peripheral was designed to act as an
infrared transceiver.
https://docs.espressif.com/projects/arduino-esp32/en/latest/api/rmt.html
************************************************************************

// Copyright 2024 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

 **
 * @brief This example demonstrates usage of RGB LED driven by RMT
 *
 * The output is a visual WS2812 RGB LED color moving in a 8 x 4 LED matrix
 * Parameters can be changed by the user. In a single LED circuit, it will just blink.
 *

void setup() {
  Serial.begin(115200);
  if (!rmtInit(BUILTIN_RGBLED_PIN, RMT_TX_MODE, RMT_MEM_NUM_BLOCKS_1, 10000000)) {
    Serial.println("init sender failed\n");
  }
  Serial.println("real tick set to: 100ns");
}

int color[] = {0x55, 0x11, 0x77};  // Green Red Blue values
int led_index = 0;

void loop() {
  // Init data with only one led ON
  int led, col, bit;
  int i = 0;
  for (led = 0; led < NR_OF_LEDS; led++) {
    for (col = 0; col < 3; col++) {
      for (bit = 0; bit < 8; bit++) {
        if ((color[col] & (1 << (7 - bit))) && (led == led_index)) {
          led_data[i].level0 = 1;
          led_data[i].duration0 = 8;
          led_data[i].level1 = 0;
          led_data[i].duration1 = 4;
        } else {
          led_data[i].level0 = 1;
          led_data[i].duration0 = 4;
          led_data[i].level1 = 0;
          led_data[i].duration1 = 8;
        }
        i++;
      }
    }
  }
  // make the led travel in the panel
  if ((++led_index) >= NR_OF_LEDS) {
    led_index = 0;
  }
  // Send the data and wait until it is done
  rmtWrite(BUILTIN_RGBLED_PIN, led_data, NR_OF_ALL_BITS, RMT_WAIT_FOR_EVER);
  delay(100);
}
***********************************************************************/

////////////////////////////////////////////////////////////////////////
#else // ESP_IDF_VERSION < 5.0.0
////////////////////////////////////////////////////////////////////////

#include "driver/rmt.h"

// Configure these based on your project needs using menuconfig ********
#define LED_RMT_TX_CHANNEL  (rmt_channel_t)0
int _PIN_LED = 8;

#define BITS_PER_LED_CMD 24
#define LED_BUFFER_ITEMS ((NUM_LEDS * BITS_PER_LED_CMD))

struct led_state {
    uint32_t leds[NUM_LEDS];
};

// These values are determined by measuring pulse timing
#define T0H 14  // the signal should be high when a 0 bit is transmitted
#define T1H 52  // the signal should be high when a 1 bit is transmitted
#define TL  52  // should be low followed by the high time for either

// This is the buffer which the hw peripheral pulsing the output pin
rmt_item32_t led_data_buffer[LED_BUFFER_ITEMS];

void setup_rmt_data_buffer(struct led_state new_state);

void ws2812_control_init(void){
    rmt_config_t config;
    config.rmt_mode = RMT_MODE_TX;
    config.channel = LED_RMT_TX_CHANNEL;
    config.gpio_num = (gpio_num_t)_PIN_LED;
    config.mem_block_num = 3;
    // glancek commented on 30 Mar 2021; not work without flag = 0
    config.flags = 0;
    config.tx_config.loop_en = false;
    config.tx_config.carrier_en = false;
    config.tx_config.idle_output_en = true;
    config.tx_config.idle_level = (rmt_idle_level_t)0;
    config.clk_div = 2;
    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
}

void ws2812_write_leds(struct led_state new_state){
    setup_rmt_data_buffer(new_state);
    ESP_ERROR_CHECK(rmt_write_items(
        LED_RMT_TX_CHANNEL, led_data_buffer, LED_BUFFER_ITEMS, false
    ));
    ESP_ERROR_CHECK(rmt_wait_tx_done(
        LED_RMT_TX_CHANNEL, portMAX_DELAY
    ));
}

void setup_rmt_data_buffer(struct led_state new_state){
    for (uint32_t led = 0; led < NUM_LEDS; led++) {
        uint32_t bits_to_send = new_state.leds[led];
        uint32_t mask = 1 << (BITS_PER_LED_CMD - 1);
        for (uint32_t bit = 0; bit < BITS_PER_LED_CMD; bit++) {
            uint32_t bit_is_set = bits_to_send & mask;
            led_data_buffer[led * BITS_PER_LED_CMD + bit]
                = bit_is_set ?
                    (rmt_item32_t){{{T1H, 1, TL, 0}}} :
                    (rmt_item32_t){{{T0H, 1, TL, 0}}};
            mask >>= 1;
        }
    }
}

// 引数r,g,bに代入された色をLEDに送信する。値は0～255の範囲で設定 //
void led(int r,int g,int b){                    // LEDにカラーを設定
    if(_PIN_LED == 0) return;
    uint32_t rgb = (g & 0xff) << 16 | (r & 0xff) << 8 | (b & 0xff);
    struct led_state new_state;
    new_state.leds[0] = rgb;
    new_state.leds[1] = rgb;
    new_state.leds[2] = rgb;
    ws2812_write_leds(new_state);
}

void led(int brightness){                       // グレースケール制御
    if(brightness > 0xff) brightness = 0xff;    // 256以上時に255に設定
    led(brightness,brightness,brightness);      // RGB全て同値でLED制御
}

void led_on(){                                  // LED制御の停止
    led(30);                                    // LEDの消灯
}

void led_off(){                                 // LED制御の停止
    led(0);                                     // LEDの消灯
}

void led_setup(int pin){
    _PIN_LED = pin;
    if(_PIN_LED == 0) return;
    Serial.println("RMT Init, JSchaenzle/ESP32-NeoPixel-WS2812-RMT, forked by Wataru KUNINO");
    Serial.print("RMT Init, ESP_IDF_VERSION: ");
    print_esp_idf_version();
    Serial.println("RMT Init, PIN="+String(pin));
    ws2812_control_init();
    led_off();
}

void led_setup(){
    led_setup(_PIN_LED);
}

#endif

    /**********************************************************************
    参考文献：
    https://github.com/JSchaenzle/ESP32-NeoPixel-WS2812-RMT
    ***********************************************************************

    2018年10月4日 時点のライセンス：

    JSchaenzle/ESP32-NeoPixel-WS2812-RMT is licensed under the

    The Unlicense

    A license with no conditions whatsoever which dedicates works to the
    public domain. Unlicensed works, modifications, and larger works may be
    distributed under different terms and without source code.

    This is free and unencumbered software released into the public domain.

    Anyone is free to copy, modify, publish, use, compile, sell, or
    distribute this software, either in source code form or as a compiled
    binary, for any purpose, commercial or non-commercial, and by any
    means.

    In jurisdictions that recognize copyright laws, the author or authors
    of this software dedicate any and all copyright interest in the
    software to the public domain. We make this dedication for the benefit
    of the public at large and to the detriment of our heirs and
    successors. We intend this dedication to be an overt act of
    relinquishment in perpetuity of all present and future rights to this
    software under copyright law.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
    OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.

    For more information, please refer to <http://unlicense.org>
    ***********************************************************************/
