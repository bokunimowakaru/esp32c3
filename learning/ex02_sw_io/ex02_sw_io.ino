/*******************************************************************************
Example 2: ESP32 (IoTセンサ) Wi-Fi ボタン
ボタンを押下するとUDPでLAN内に文字列"Ping"を送信します。
LINE用のトークンを設定すれば、LINEアプリに「ボタンが押されました」を通知します。
別の子機となる Wi-Fi コンシェルジェ証明担当（ワイヤレスLED子機）のIPアドレスを
設定すれば、ボタンを押下したときにLEDをON、押し続けたときにOFFに制御します。

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
    7. 下記のLINE_TOKENのダブルコート(")内に貼り付け
 *****************************************************************************/
#define LINE_TOKEN  "your_token"                // LINE Notify トークン★要設定

/******************************************************************************
 Wi-Fi コンシェルジェ証明担当（ワイヤレスLED子機） の設定
 ******************************************************************************
 ※ex01_led または ex01_led_io が動作する、別のESP32C3搭載デバイスが必要です
    1. ex01_led/ex01_led_io搭載デバイスのシリアルターミナルでIPアドレスを確認
    2. 下記のLED_IPのダブルコート(")内に貼り付け
 *****************************************************************************/
#define LED_IP "192.168.1.0"                    // LED搭載子機IPアドレス★要設定

#define PIN_LED 0                               // IO 0 に (通常の)LED を接続
#define PIN_SW 1                                // IO1 にタクトスイッチを接続
#define SSID "1234ABCD"                         // 無線LANアクセスポイント SSID
#define PASS "password"                         // パスワード
#define PORT 1024                               // 送信のポート番号

IPAddress IP_BROAD;                             // ブロードキャストIPアドレス
int wake = (int)esp_sleep_get_wakeup_cause();   // 起動理由を変数wakeに保存

void setup(){                                   // 起動時に一度だけ実行する関数
    pinMode(PIN_LED, OUTPUT);                   // (通常の)LED用IOポートを出力に
    pinMode(PIN_SW,INPUT_PULLUP);               // タクトスイッチ入力の設定
    Serial.begin(115200);                       // 動作確認のためのシリアル出力
    Serial.println("ESP32C3 SW UDP LINE LED");  // 「SW UDP」をシリアル出力表示
    Serial.print(" Wake = ");                   // 「wake =」をシリアル出力表示
    Serial.println(wake);                       // 起動理由noをシリアル出力表示
    if(wake != 7) sleep();                      // ボタン以外で起動時にスリープ

    WiFi.mode(WIFI_STA);                        // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                      // 無線LANアクセスポイント接続
    while(WiFi.status() != WL_CONNECTED){       // 接続に成功するまで待つ
        digitalWrite(PIN_LED, millis()/100%2);  // (通常の)LEDの点滅
        if(millis() > 30000) sleep();           // 30秒超過でスリープ
        delay(50);                              // 待ち時間処理
    }
    digitalWrite(PIN_LED, HIGH);                // (通常の)LEDを点灯
    IP_BROAD = WiFi.localIP();                  // IPアドレスを取得
    IP_BROAD[3] = 255;                          // ブロードキャストアドレスに
    Serial.println(IP_BROAD);                   // ブロードキャストアドレス表示
}

void loop(){                                    // 繰り返し実行する関数
    WiFiUDP udp;                                // UDP通信用のインスタンス定義
    udp.beginPacket(IP_BROAD, PORT);            // UDP送信先を設定
    udp.println("Ping");                        // メッセージ"Ping"を送信
    udp.endPacket();                            // UDP送信の終了(実際に送信)
    delay(200);                                 // 送信待ち時間

    HTTPClient http;                            // HTTPリクエスト用インスタンス
    http.setConnectTimeout(15000);              // タイムアウトを15秒に設定する
    String url;                                 // URLを格納する文字列変数を生成
    if(strlen(LINE_TOKEN) > 42){                // LINE_TOKEN設定時
        url = "https://notify-api.line.me/api/notify";  // LINEのURLを代入
        Serial.println(url);                    // 送信URLを表示
        http.begin(url);                        // HTTPリクエスト先を設定する
        http.addHeader("Content-Type","application/x-www-form-urlencoded");
        http.addHeader("Authorization","Bearer " + String(LINE_TOKEN));
        http.POST("message=ボタンが押されました"); // メッセージをLINEへ送信する
        http.end();                             // HTTP通信を終了する
    }
    if(strcmp(LED_IP,"192.168.1.0")){           // 子機IPアドレス設定時
        url = "http://" + String(LED_IP) + "/?L="; // アクセス先URL
        url += String(digitalRead(PIN_SW));     // ボタン開放時にLEDをON
        Serial.println(url);                    // 送信URLを表示
        http.begin(url);                        // HTTPリクエスト先を設定する
        http.GET();                             // ワイヤレスLEDに送信する
        http.end();                             // HTTP通信を終了する
    }
    sleep();                                    // 下記のsleep関数を実行
}


void sleep(){                                   // スリープ実行用の関数
    Serial.println(digitalRead(PIN_SW));        // タクトスイッチ状態を表示
    int i = 0;                                  // ループ用の数値変数i
    while(i<100){                               // スイッチ・ボタン解除待ち
        i = digitalRead(PIN_SW) ? i+1 : 0;      // ボタン開放時にiに1を加算
        delay(1);                               // 待ち時間処理
    }
    digitalWrite(PIN_LED, LOW);                 // (通常の)LEDを消灯
    Serial.println("Sleep...");                 // 「Sleep」をシリアル出力表示
    delay(100);                                 // 待ち時間処理
    uint64_t pin = 1ULL << PIN_SW;              // 起動用IOポートのマスク作成
    esp_deepsleep_gpio_wake_up_mode_t val = ESP_GPIO_WAKEUP_GPIO_LOW;
    esp_deep_sleep_enable_gpio_wakeup(pin,val); // スリープ解除設定
    esp_deep_sleep_start();                     // Deep Sleepモードへ移行
}
