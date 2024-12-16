/***********************************************************************
led_ws2812 RGB LED WS2812
                                   Copyright (c) 2021-2024 Wataru KUNINO
***********************************************************************/

#define PIN_LED 2                   // GPIO 2 に WS2812 を接続(m5stamp用)
// #define PIN_LED 8                // GPIO 8 に WS2812 を接続(DevKitM用)

void led_setup(int);
void led(int);
void led(int,int,int);
byte ledp[][3]={{10,10,10},{20,5,5},{5,20,5},{5,5,20}};

/* 初期化処理 */
void setup() {                  // 起動時に一度だけ実行される関数
    Serial.begin(115200);
    Serial.println("RGB LED WS2812");
    led_setup(PIN_LED);
    led(0,0,0);
}

/* LEDの点滅処理 */
void loop() {                   // setup実行後に繰り返し実行される関数
    for(int i=0;i<sizeof(ledp)/sizeof(ledp[0]);i++){
        Serial.printf("p%d={%d,%d,%d}\n",i,ledp[i][0],ledp[i][1],ledp[i][2]);
        led(ledp[i][0],ledp[i][1],ledp[i][2]);
        delay(1000);            // 時間待ち(1秒)
    }
}

rmt_data_t led_data[24];
int _PIN_LED = 0;

void led(int r,int g,int b){                    // LEDにカラーを設定
    if(_PIN_LED == 0) return;
    uint32_t rgb = (g & 0xff) << 16 | (r & 0xff) << 8 | (b & 0xff);
    for (int bit = 0; bit < 24; bit++) {
        if ((rgb & (1 << (23 - bit))) ) {
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
    rmtWrite(_PIN_LED, led_data, 24, RMT_WAIT_FOR_EVER);
}

void led(int brightness){                       // グレースケール制御
    if(brightness > 0xff) brightness = 0xff;    // 256以上時に255に設定
    led(brightness,brightness,brightness);      // RGB全て同値でLED制御
}

void led_setup(int pin){
    _PIN_LED = pin;
    rmtInit(_PIN_LED, RMT_TX_MODE, RMT_MEM_NUM_BLOCKS_1, 10000000);
    // Serial.println("real tick set to: 100ns");
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
