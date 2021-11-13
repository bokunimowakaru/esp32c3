/*******************************************************************************
Practice esp32 14 pir 【Wi-Fi 人感センサ子機】ディープスリープ版

・回路図はESP書 P.75を参照し、以下に留意
・人感センサの出力インピーダンスが低い。FETかトランジスタで反転させIO 10に入力
・人感センサからESP32を起動するための信号は,ESP32のEN入力またはPIN_WAKEに入力
　※PIN_WAKEを使用する場合は回路図から変更が必要
　　（ここでは、一例として、ESP32C3 IO4に接続する）
・LEDはIO2に接続

                                           Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/
#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include "esp_sleep.h"                      // ESP32用Deep Sleep ライブラリ
#define PIN_LED 2                           // IO 2にLEDを接続
#define PIN_PIR 10                          // IO 10にPIRセンサを接続
#define PIN_WAKE 4                          // IO 4にPIRセンサの起動信号を接続
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define PORT 1024                           // 受信ポート番号
#define SLEEP_P 3550ul*1000000ul            // スリープ時間 3550秒(約60分)
#define DEVICE "pir_s_1,"                   // デバイス名(5文字+"_"+番号+",")
boolean pir;                                // 人感センサ値
IPAddress IP;                               // ブロードキャストIP保存用

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    digitalWrite(PIN_LED,1);                // LEDを点灯
    pinMode(PIN_PIR,INPUT);                 // センサを接続したポートを入力に
    pinMode(PIN_WAKE,INPUT_PULLUP);         // 起動信号を接続したポートを入力に
    pir=!digitalRead(PIN_PIR);              // 人感センサの状態を取得
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32 Example 06 PIR"); // 「Example 06 PIR」をシリアル出力
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        Serial.print('.');                  // 進捗表示
        delay(100);                         // 待ち時間
        if(millis() > 30000) sleep();       // 30000ms(30秒)を過ぎたらスリープ
    }
    digitalWrite(PIN_LED,1);
    IP = WiFi.localIP();                    // IPアドレスを取得
    Serial.println(IP);
    IP[3] = 255;                            // ブロードキャストアドレスに
}

void loop(){                                // 繰り返し実行する関数
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    udp.beginPacket(IP, PORT);              // UDP送信先を設定
    udp.print(DEVICE);                      // デバイス名を送信
    udp.print(pir);                         // 起動直後のセンサ状態を送信
    udp.print(", ");                        // 「,」カンマと「␣」を送信
    pir=!digitalRead(PIN_PIR);              // 人感センサの状態を取得
    udp.println(pir);                       // 現在のセンサの状態を送信
    Serial.println(pir);                    // シリアル出力表示
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    delay(10);                              // 送信待ち時間
    sleep();                                // sleepを実行
}

void sleep(){
    while(!digitalRead(PIN_WAKE)) delay(100); // 起動信号がHレベルなるまで待機
    esp_deep_sleep_enable_gpio_wakeup(1ul<<PIN_WAKE,ESP_GPIO_WAKEUP_GPIO_LOW);
    Serial.println("Going to sleep, now...");
    delay(200);                             // 送信待ち時間
    esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
    delay(5000);
    return;
}

/*
SB412A(NANYANG SENBA OPTICAL AND ELECTRONIC CO. LTD.)
1.VIN		3.5～12V (実力的は3.3Vでも動作。レギュレータは3.0V)
2.VOUT		Output: High level signal 3V ⇒ FETかトランジスタで受けてから使用
3.GND

BS612 AS612(NANYANG SENBA OPTICAL AND ELECTRONIC CO. LTD.)
1.SENS		感度設定 0.12V 感度2（０～３１）
2.OEN		2.367
3.VSS
4.VDD
5.REL		出力 ⇒ FETかトランジスタで受けてから使用する
6.ONTIME	ホールド時間設定 0.48V 30秒程度

*/
