/***********************************************************************
led02_dimmer RGB LED WS2812
フルカラーLED の 輝度を制御
                                        Copyright (c) 2021 Wataru KUNINO
***********************************************************************/

#define PIN_LED 2                   // GPIO 2 に WS2812 を接続(m5stamp用)
// #define PIN_LED 8                // GPIO 8 に WS2812 を接続(DevKitM用)

/* 初期化処理 */
void setup() {                                  // 一度だけ実行する関数
    led_setup(PIN_LED);                         // LED接続ポートを出力に
}

/* LEDの制御 */
int brightness = 0;                             // 現在の輝度値
int dimmer_speed = +1;                          // 輝度の変更速度
int dimmer_max = 30;                            // 点灯時の輝度(255以下)
void loop() {                                   // 繰り返し実行する関数
    led(brightness);                            // LED 制御
    brightness += dimmer_speed;                 // 輝度の増減
    if(brightness < 0){                         // 輝度値が負になったとき
        brightness = 0;                         // 輝度値を0に設定
        dimmer_speed = abs(dimmer_speed);       // 正の速度を設定
        delay(257);                             // 257msの待ち時間
    }else if(brightness > dimmer_max){          // 点灯時の輝度を超えたとき
        brightness = dimmer_max;                // 転倒時の輝度を設定
        dimmer_speed = -abs(dimmer_speed);      // 負の速度
    }
    delay(10);                                  // 10msの待ち時間処理
}
