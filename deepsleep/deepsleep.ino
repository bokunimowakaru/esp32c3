/*******************************************************************************
ESP32-C3 乾電池での動作に必要な Deep Sleep の実験用サンプル・プログラムです。
解説＝ https://bokunimo.net/blog/esp/1551/

                                               Copyright (c) 2021 Wataru KUNINO
*******************************************************************************/
#include "esp_sleep.h"                      // ESP32用Deep Sleep ライブラリ
#define PIN_LED 2                           // GPIO 2 に WS2812 を接続(m5stamp用)
// #define PIN_LED 8                        // GPIO 8 に WS2812 を接続(DevKitM用)
#define PIN_SW 9                            // スリープ実行ボタン
#define PIN_SW_HOLDING 3                    // スリープ実行用ボタン押下時間(3秒)
#define PIN_WAKE 4                          // スリープ解除ボタン
#define SLEEP_P 30 * 1000000ul              // スリープから復帰するまでの時間(30秒)

void setup(){                               // 起動時に一度だけ実行する関数
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    TimerWakeUp_init();                     // 起動理由の表示
    Serial.println("ESP32: Hello!");        // 「ESP32 Hello」をシリアル出力表示
    pinMode(PIN_SW,INPUT_PULLUP);           // ボタン入力の設定
    led_setup(PIN_LED);
    Serial.println("Hold PIN_SW("+String(PIN_SW)+") to sleep");
}

void loop() {
    int i=1,j=0,a=+1;
    while(i > 0){
        if(digitalRead(PIN_SW) && !digitalRead(PIN_WAKE)){
            i = PIN_SW_HOLDING * 20 + 1;
            led(j);
            j+=a;
            if(j>10) a=-1;
            if(j<=0) a=+1;
        }else led(5);
        if(i%20 == 0) Serial.println(
            "PIN_SW("+String(PIN_SW)+")="+String(digitalRead(PIN_SW))+
            ", PIN_WAKE("+String(PIN_WAKE)+")="+String(digitalRead(PIN_WAKE))+
            ", Sec."+String(i/20)
        );
        i--;
        delay(50);                          // 待ち時間
    }
    Serial.println("Going to sleep, now...");
    led_off();
    delay(100);
    Serial.flush();
    TimerWakeUp_setExternalInput(PIN_WAKE,1); // スリープ解除ピンがHで起動 
    esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
}
