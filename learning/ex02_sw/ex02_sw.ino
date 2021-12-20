/*******************************************************************************
Example 2: ESP32 (IoTセンサ) Wi-Fi ボタン
ボタンを押下するとUDPでLAN内に文字列"Ping"を送信します。
LINE用のトークンを設定すれば、LINEアプリに「ボタンが押されました」を通知します。

                                          Copyright (c) 2021-2022 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                               // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                            // UDP通信を行うライブラリ
#include <HTTPClient.h>                         // HTTPクライアント用ライブラリ
#include "esp_sleep.h"                          // ESP32用Deep Sleep ライブラリ

/******************************************************************************
 LINE Notify 設定
 ******************************************************************************
 ※LINE アカウントと LINE Notify 用のトークンが必要です。
    1. https://notify-bot.line.me/ へアクセス
    2. 右上のアカウントメニューから「マイページ」を選択
    3. トークン名「esp32」を入力
    4. 送信先のトークルームを選択する(「1:1でLINE Notifyから通知を受け取る」等)
    5. [発行する]ボタンでトークンが発行される
    6. [コピー]ボタンでクリップボードへコピー
    7. 下記のLINE_TOKENに貼り付け
 *****************************************************************************/
#define LINE_TOKEN  "your_token"                // LINE Notify トークン★要設定
#define PIN_SW 3                                // IO1にボタンを接続
#define PIN_LED_RGB 2                           // IO2 に WS2812を接続(m5stamp)
// #define PIN_LED_RGB 8                        // IO8 に WS2812を接続(DevKitM)
#define SSID "1234ABCD"                         // 無線LANアクセスポイント SSID
#define PASS "password"                         // パスワード
#define PORT 1024                               // 送信のポート番号

IPAddress IP_BROAD;                             // ブロードキャストIPアドレス
int wake = (int)esp_sleep_get_wakeup_cause();   // 起動理由を変数wakeに保存

void setup(){                                   // 起動時に一度だけ実行する関数
    led_setup(PIN_LED_RGB);                     // WS2812の初期設定(ポート設定)
    Serial.begin(115200);                       // 動作確認のためのシリアル出力
    Serial.println("ESP32C3 SW UDP + toLINE");  // 「SW UDP」をシリアル出力表示
    Serial.println(wake);
    if(wake != 7) sleep();                      // ボタン以外で起動時にスリープ

    WiFi.mode(WIFI_STA);                        // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                      // 無線LANアクセスポイント接続
    while(WiFi.status() != WL_CONNECTED){       // 接続に成功するまで待つ
        delay(50);                              // 待ち時間処理
        led((millis()/50) % 10);                // WS2812の点滅
        if(millis() > 30000) sleep();           // 30秒超過でスリープ
    }
    led(0,20,0);                                // LEDを緑色で点灯
    IP_BROAD = WiFi.localIP();                  // IPアドレスを取得
    IP_BROAD[3] = 255;                          // ブロードキャストアドレスに
    Serial.println(IP_BROAD);                   // ブロードキャストアドレス表示
}

void loop(){
    WiFiUDP udp;                                // UDP通信用のインスタンス定義
    udp.beginPacket(IP_BROAD, PORT);            // UDP送信先を設定
    udp.println("Ping");                        // メッセージ"Ping"を送信
    udp.endPacket();                            // UDP送信の終了(実際に送信)
    delay(200);                                 // 送信待ち時間
    if(strlen(LINE_TOKEN) < 43) sleep();        // LINE_TOKEN未設定時にsleep

    HTTPClient http;                            // HTTPリクエスト用インスタンス
    http.begin("https://notify-api.line.me/api/notify");    // アクセス先URL
    http.addHeader("Content-Type","application/x-www-form-urlencoded");
    http.addHeader("Authorization","Bearer " + String(LINE_TOKEN));
    http.POST("message=ボタンが押されました");  // メッセージをLINEへ送信する
    sleep();                                    // 下記のsleep関数を実行
}

void sleep(){
    pinMode(PIN_SW,INPUT_PULLUP);               // ボタン入力の設定
    Serial.println(digitalRead(PIN_SW));        // ボタン状態をシリアル表示
    int i = 0;                                  // ループ用の数値変数i
    while(i<100) i = digitalRead(PIN_SW)?i+1:0; // ボタン解除待ちループ
    led_off();                                  // WS2812の消灯
    delay(100);                                 // 待ち時間処理
    unsigned long long pin = 1ULL << PIN_SW;	// 起動用IOポートのマスク作成
    esp_deepsleep_gpio_wake_up_mode_t val = ESP_GPIO_WAKEUP_GPIO_LOW;
    esp_deep_sleep_enable_gpio_wakeup(pin,val); // スリープ解除設定
    esp_deep_sleep_start();                     // Deep Sleepモードへ移行
}
