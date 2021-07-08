/*******************************************************************************
Example 36(=32+4): ESP32 ケチケチ運転術
乾電池などで動作するIoTセンサ用の基本形です。

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/
#include "esp_sleep.h"                      // ESP32用Deep Sleep ライブラリ
#define PIN_SW 9
#define SLEEP_P 5 * 1000000ul

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_SW,INPUT_PULLUP);           // ボタン入力の設定
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32 eg.04 LE");       // 「ESP32 eg.04」をシリアル出力表示
}

void loop() {
    for(int i = 50; i > 0; i--){
        if(!digitalRead(PIN_SW)) i = 50;
        if(i%10 == 0) Serial.println(i/10);
        delay(100);                         // 送信待ち時間
    }
    esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
}
