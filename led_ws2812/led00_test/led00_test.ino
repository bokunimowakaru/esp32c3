/***********************************************************************
led_ws2812 RGB LED WS2812
                                        Copyright (c) 2021 Wataru KUNINO
***********************************************************************/
/*
参考文献
WS2812B Ver. No.: V5, Intelligent control LED, integrated light source
http://www.world-semi.com/
https://akizukidenshi.com/download/ds/worldsemi/WS2812B_20200225.pdf
https://www.rose-lighting.com/wp-content/uploads/sites/53/2020/05/SK68XX-MINI-HS-REV.04-EN23535RGB-thick.pdf
*/

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

/*
#define ESP32C3

#define T_DELAY 360
#ifdef ESP32C3
    #define T0H_ns (320+400)/2
    #define T0L_ns 1250 - T0H_ns
    #define T1H_ns (640+1000)/2
    #define T1L_ns 1250 - T1H_ns
#endif
#ifdef WS2812
    #define T0H_ns (220+380)/2
    #define T0L_ns (580+1000)/2
    #define T1H_ns (580+1000)/2
    #define T1L_ns (580+1000)/2
#endif
#ifdef SK68XXMINI
    #define T0H_ns 320                          // b0信号Hレベル時間(ns)
    #define T0L_ns 1200 -320                    // b0信号Lレベル時間(ns)
    #define T1H_ns 640                          // b1信号Hレベル時間(ns)
    #define T1L_ns 1200 -640                    // b1信号Lレベル時間(ns)
#endif

int T_Delay = T_DELAY;
int T0H_num = 0;
int T0L_num = 8;
int T1H_num = 4;
int T1L_num = 3;
*/


/* 引数nsに代入された待ち時間(ns)に対応する待ち時間処理回数を求める */
/*
int _led_delay(uint32_t ns){                    // ns -> num 変換用
    volatile uint32_t i;
    uint32_t start, end, t0, t1, counts, num;

    delay(100);
    // noInterrupts();
    i = 100;                                    // 空ループの測定回数
    delay(1);
    start = micros();                           // 開始時刻の保持
    while(i>0) i--;                             // 100回の空ループ
    end = micros();                             // 終了時刻の保持
    t0 = (end - start) * 10;                    // 空ループの処理時間
    // interrupts();
    Serial.printf("target=%d ns, t0=%d ns, ", ns, t0);
    if(ns > T_Delay + t0){                      // 減算可能なとき
        ns -= T_Delay + t0;                     // IO制御遅延を減算
    }else{                                      // 可能でないとき
        ns =0;                                  // 0にする
    }

    delay(100);
    // noInterrupts();
    counts = 0;
    delay(1);
    do{
        i = ++counts;
        start = micros();
        while(i>0) i--;
        end = micros();
    }while(end < start + ns /10);
    // interrupts();
    t1 = (end - start) * 10;
    num = (counts + 50) / 100;
    Serial.printf("t1=%d ns, t2=%d ns, num=%d\n", t1, T_Delay + t0 + t1, num);
    return num;

}
*/

/* 信号操作に必要な処理時間を算出する。戻り値は必要時間(ns) */
/*
int _initial_delay(){                           // IO制御ディレイ測定部
    volatile uint32_t i=0;                      // 繰り返し処理用変数i
    uint32_t start, end, t, counts;             // 開始時刻,試行繰返し数

    delay(100);
    // noInterrupts();                          // 割り込みの禁止
    counts = 0;                                 // カウンタのリセット
    delay(1);
    start = micros();                           // 開始時刻の保持
    do{                                         // 繰り返し処理の開始
        counts++;                               // カウント
    }while(counts < 100);                       // 目標未達成時に繰返し
    end = micros();                             // 終了時刻の保持
    // interrupts();
    t = end - start;                            // 経過時間をtに代入

    delay(100);
    // noInterrupts();
    counts = 0;                                 // カウンタのリセット
    delay(1);
    start = micros();                           // 開始時刻の保持
    do{                                         // 繰り返し処理の開始
        counts++;                               // カウント
        digitalWrite(PIN_LED,HIGH);             // (被測定対象)GPIO制御
        while(i>0) i=0;                         // (被測定対象)while
    }while(counts < 100);                       // 目標未達成時に繰返し
    end = micros();                             // 終了時刻の保持
    t = end - start - t;                        // 経過時間をtに代入
    // interrupts();                            // 割り込みの許可
    return t * 10;                              // 繰り返し回数を応答
}

void _led_reset(){
    digitalWrite(PIN_LED,LOW);
    delayMicroseconds(300);                     // 280us以上
}
*/

/* 引数r,g,bに代入された色をLEDに送信する。値は0～255の範囲で設定 */
/*
void led(int r,int g,int b){
    _led_reset();
    volatile uint32_t TH, TL;                   // H/Lレベル時間保持用
    uint32_t rgb = (g & 0xff) << 16 | (r & 0xff) << 8 | (b & 0xff);
    // noInterrupts();                          // 割り込みの禁止
    for(int b=23;b >= 0; b--){                  // 全24ビット分の処理
        if(rgb & (1<<b)){                       // 対象ビットが1のとき
            TH = T1H_num;                       // Hレベルの待ち時間設定
            TL = T1L_num;                       // Hレベルの待ち時間設定
        }else{                                  // 対象ビットが0のとき
            TH = T0H_num;                       // Lレベルの待ち時間設定
            TL = T0L_num;                       // Lレベルの待ち時間設定
        }
        if(TH){
            digitalWrite(PIN_LED,HIGH);         // Hレベルを出力
            while(TH>0) TH--;                   // 待ち時間処理
            digitalWrite(PIN_LED,LOW);          // Lレベルを出力
            while(TL>0) TL--;                   // 待ち時間処理
        }else{
            digitalWrite(PIN_LED,HIGH);         // Hレベルを出力
            digitalWrite(PIN_LED,LOW);          // Lレベルを出力
            while(TL>0) TL--;                   // 待ち時間処理
        }

    }
    // interrupts();
}
*/

/* 初期化処理 */
/*
void setup() {                  // 起動時に一度だけ実行される関数
    Serial.begin(115200);
    Serial.println("RGB LED WS2812");
    pinMode(PIN_LED,OUTPUT);    // LEDを接続したポートを出力に設定する
    digitalWrite(PIN_LED,HIGH);
    T_Delay = _initial_delay();
    Serial.printf("T_Delay = %dns\n",T_Delay);
    T0H_num=_led_delay(T0H_ns);
    T0L_num=_led_delay(T0L_ns);
    T1H_num=_led_delay(T1H_ns);
    T1L_num=_led_delay(T1L_ns);
    Serial.printf("T0H %dns -> %d, ",T0H_ns+T_Delay,T0H_num);
    Serial.printf("T0L %dns -> %d\n",T0L_ns+T_Delay,T0L_num);
    Serial.printf("T1H %dns -> %d, ",T1H_ns+T_Delay,T1H_num);
    Serial.printf("T1L %dns -> %d\n",T1L_ns+T_Delay,T1L_num);
    led(0,0,0);
}
*/

/***********************************************************************
LED制御ドライバ RGB LED WS2812
                                        Copyright (c) 2022 Wataru KUNINO
************************************************************************
MITライセンスで配布します。権利表示の改変は禁止します。全て無保証です。
本ソースコードには末尾に示すライセンス(The Unlicense)に基づいたコードを
含みます。
***********************************************************************/

#include "driver/rmt.h"

// Configure these based on your project needs using menuconfig ********
#define LED_RMT_TX_CHANNEL  (rmt_channel_t)0
int _PIN_LED = 8;

#define NUM_LEDS 3
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

/* 引数r,g,bに代入された色をLEDに送信する。値は0～255の範囲で設定 */
void led(int r,int g,int b){                    // LEDにカラーを設定
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
    ws2812_control_init();
    led_off();
}

void led_setup(){
    led_setup(_PIN_LED);
}

/******************************************************************************
lib_led.ino
*******************************************************************************
MIT License

Copyright (c) 2022 Wataru KUNINO

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

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
