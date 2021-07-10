/*******************************************************************************
ESP32-C3 乾電池での動作に必要な Deep Sleep の実験用サンプル・プログラムです。
解説＝ https://bokunimo.net/blog/esp/1551/

                                               Copyright (c) 2021 Wataru KUNINO
*******************************************************************************/
#include "esp_sleep.h"                      // ESP32用Deep Sleep ライブラリ
#define PIN_SW 9                            // スリープ実行ボタン
#define PIN_SW_HOLDING 3                    // スリープ実行用ボタン押下時間(3秒)
#define SLEEP_P 10 * 1000000ul              // スリープから復帰するまでの時間(10秒)

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_SW,INPUT_PULLUP);           // ボタン入力の設定
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32: Hello!");        // 「ESP32 eg.04」をシリアル出力表示
    TimerWakeUp_print_wakeup_reason();      // 起動理由の表示
    led_setup();
}

void loop() {
    int i=1,j=0,a=+1;
    while(i > 0){
        if(digitalRead(PIN_SW)){
            i = PIN_SW_HOLDING * 20 + 1;
            led(j);
            j+=a;
            if(j>10) a=-1;
            if(j<=0) a=+1;
        }else led(5);
        if(i%20 == 0) Serial.println(i/20);
        i--;
        delay(50);                          // 待ち時間
    }
    Serial.println("Going to sleep...");
    led_off();
    delay(100);
    Serial.flush();
    esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
}
