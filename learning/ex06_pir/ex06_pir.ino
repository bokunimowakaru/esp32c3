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
#include <HTTPClient.h>                         // HTTPクライアント用ライブラリ
#include "esp_sleep.h"                      // ESP32用Deep Sleep ライブラリ

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
    1. ex01_led/ex01_led_io搭載デバイスを実行し、シリアルターミナルでIPアドレスを確認する
    2. 下記のLED_IPのダブルコート(")内に貼り付け
 *****************************************************************************/
#define LED_IP "192.168.1.0"                    // LED搭載子機のIPアドレス★要設定

#define PIN_PIR 1                               // IO 1にセンサ(人感/ドア)を接続
#define PIN_LED_RGB 2                           // IO2 に WS2812を接続(m5stamp)
// #define PIN_LED_RGB 8                        // IO8 に WS2812を接続(DevKitM)
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define PORT 1024                           // 受信ポート番号
#define DEVICE "pir_s_1,"                   // 人感センサ時デバイス名
// #define DEVICE "rd_sw_1,"                // ドアセンサ時デバイス名

RTC_DATA_ATTR boolean PIR;                      // pir値のバックアップ保存用
boolean pir;                                    // 人感センサ値 or ドアセンサ状態値
esp_deepsleep_gpio_wake_up_mode_t pir_wake;     // 起動用のpir値
IPAddress IP_BROAD;                             // ブロードキャストIPアドレス
int wake = (int)esp_sleep_get_wakeup_cause();   // 起動理由を変数wakeに保存

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_PIR,INPUT);                 // センサを接続したポートを入力に
    pir = digitalRead(PIN_PIR);              // 人感センサの状態を取得
    led_setup(PIN_LED_RGB);                     // WS2812の初期設定(ポート設定)

    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32C3 PIR/Reed");         // 「ESP32C3 PIR/Reed」を出力
    Serial.print(" Wake = ");                   // 「wake =」をシリアル出力表示
    Serial.println(wake);                       // 起動理由noをシリアル出力表示
    if(wake == 0) PIR = pir;                    // ボタン以外で起動時にPIR値を設定
    if(wake != 7) sleep();                      // ボタン以外で起動時にスリープ

    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        led((millis()/50) % 10);                // (WS2812)LEDの点滅
        if(millis() > 30000) sleep();           // 30秒超過でスリープ
        delay(50);                              // 待ち時間処理
    }
    led(0,20,0);                                // (WS2812)LEDを緑色で点灯
    IP_BROAD = WiFi.localIP();                  // IPアドレスを取得
    IP_BROAD[3] = 255;                          // ブロードキャストアドレスに
    Serial.println(IP_BROAD);                   // ブロードキャストアドレス表示
}

void loop(){                                // 繰り返し実行する関数
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    udp.beginPacket(IP_BROAD, PORT);            // UDP送信先を設定
    udp.print(DEVICE);                      // デバイス名を送信
    udp.print(PIR);                         // 起動時のセンサ状態を送信
    udp.print(", ");                        // 「,」カンマと「␣」を送信
    pir = digitalRead(PIN_PIR);              // 人感センサの状態を取得
    udp.println(pir);                       // 現在のセンサの状態を送信
    Serial.println(pir);                    // シリアル出力表示
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    delay(10);                              // 送信待ち時間

    HTTPClient http;                            // HTTPリクエスト用インスタンス
    http.setConnectTimeout(15000);              // タイムアウトを15秒に設定する
    String url;                                 // URLを格納する文字列変数を生成
    if(strlen(LINE_TOKEN) > 42){                // LINE_TOKEN設定時
        url = "https://notify-api.line.me/api/notify";  // LINEのURLを代入
        Serial.println(url);                    // 送信URLを表示
        http.begin(url);                        // HTTPリクエスト先を設定する
        http.addHeader("Content-Type","application/x-www-form-urlencoded");
        http.addHeader("Authorization","Bearer " + String(LINE_TOKEN));
        http.POST("message=センサが反応しました。Value=" + String(pir)); // メッセージをLINEへ送信する
        http.end();                             // HTTP通信を終了する
    }
    sleep();                                // sleepを実行
}

void sleep(){
    int i = 0;                                  // ループ用の数値変数i
    while(i<100){                               // スイッチ・ボタン解除待ち
        boolean pir_b = digitalRead(PIN_PIR);
        if( pir == pir_b){
            i++;
        }else{
            i = 0;
            pir = pir_b;
        }
        delay(1);                               // 待ち時間処理
    }
    PIR = !pir;
    if(PIR) pir_wake = ESP_GPIO_WAKEUP_GPIO_HIGH;
    else    pir_wake = ESP_GPIO_WAKEUP_GPIO_LOW;

    Serial.println("Going to sleep, now...");
    delay(100);                                 // 待ち時間処理
    unsigned long long pin = 1ULL << PIN_PIR;	// 起動用IOポートのマスク作成
    pin |= 1ULL << PIN_PIR;	                    // 起動用IOポートのマスク作成
    esp_deep_sleep_enable_gpio_wakeup(1ul<<PIN_PIR, pir_wake);
    esp_deep_sleep_start();                     // Deep Sleepモードへ移行
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
