/*******************************************************************************
Example 5: ESP32C3 (IoTセンサ) Wi-Fi 温湿度計 SENSIRION製 SHT31/SHT35 版
デジタルI2Cインタフェース搭載センサから取得した温湿度を送信するIoTセンサです。

    ESP32 のI2Cポート:
        SHT31/SHT35 SDAポート GPIO 1
        SHT31/SHT35 SCLポート GPIO 0    設定方法＝shtSetup(SDA,SCL)

                                          Copyright (c) 2016-2022 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                               // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                            // UDP通信を行うライブラリ
#include <HTTPClient.h>                         // HTTPクライアント用ライブラリ
#include "esp_sleep.h"                          // ESP32用Deep Slee2022p ライブラリ

#define PIN_LED_RGB 2                           // IO2 に WS2812を接続(m5stamp)
// #define PIN_LED_RGB 8                        // IO8 に WS2812を接続(DevKitM)
#define SSID "1234ABCD"                         // 無線LANアクセスポイントのSSID
#define PASS "password"                         // パスワード
#define PORT 1024                               // 送信のポート番号
#define SLEEP_P 30*1000000ul                    // スリープ時間 30秒(uint32_t)
#define DEVICE "humid_1,"                       // デバイス名(5文字+"_"+番号+",")

/******************************************************************************
 Ambient 設定
 ******************************************************************************
 ※Ambientでのアカウント登録と、チャネルID、ライトキーを取得する必要があります。
    1. https://ambidata.io/ へアクセス
    2. 右上の[ユーザ登録(無料)]ボタンでメールアドレス、パスワードを設定してアカウントを登録
    3. [チャネルを作る]ボタンでチャネルIDを新規作成する
    4. 「チャネルID」を下記のAmb_Idのダブルコート(")内に貼り付ける
    5. 「ライトキー」を下記のAmb_Keyに貼り付ける
 (参考文献) IoTデータ可視化サービスAmbient(アンビエントデーター社) https://ambidata.io/
*******************************************************************************/
#define Amb_Id  "00000"                         // AmbientのチャネルID 
#define Amb_Key "0000000000000000"              // Ambientのライトキー

IPAddress IP_BROAD;                             // ブロードキャストIPアドレス

void setup(){                                   // 起動時に一度だけ実行する関数
    led_setup(PIN_LED_RGB);                     // WS2812の初期設定(ポート設定)
    shtSetup(1,0);                              // 湿度センサの初期化
    Serial.begin(115200);                       // 動作確認のためのシリアル出力開始
    Serial.println("ESP32C3 HUM");              // 「ESP32C3 HUM」をシリアル出力
    
    WiFi.mode(WIFI_STA);                        // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                      // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){       // 接続に成功するまで待つ
        led((millis()/50) % 10);                // (WS2812)LEDの点滅
        if(millis() > 30000) sleep();           // 30秒超過でスリープ
        delay(50);                              // 待ち時間処理
    }
    led(0,20,0);                                // (WS2812)LEDを緑色で点灯
    IP_BROAD = WiFi.localIP();                  // IPアドレスを取得
    IP_BROAD[3] = 255;                          // ブロードキャストアドレスに
    Serial.println(IP_BROAD);                   // ブロードキャストアドレス表示
}

void loop(){
    float temp = getTemp();                     // 温度を取得して変数tempに代入
    float hum =getHum();                        // 湿度を取得して変数humに代入
    if(temp < -100. || hum < 0.) sleep();       // 取得失敗時に末尾のsleepを実行

    String S = String(DEVICE);                  // 送信データSにデバイス名を代入
    S += String(temp,1) + ", ";                 // 送信データSに変数tempの値を追記
    S += String(hum,1);                         // 送信データSに変数humの値を追記
    Serial.println(S);                          // 送信データSをシリアル出力表示
    WiFiUDP udp;                                // UDP通信用のインスタンスを定義
    udp.beginPacket(IP_BROAD, PORT);            // UDP送信先を設定
    udp.println(S);                             // 送信データSをUDP送信
    udp.endPacket();                            // UDP送信の終了(実際に送信する)
    if(strcmp(Amb_Id,"00000") == 0) sleep();    // Ambient未設定時にsleepを実行

    S = "{\"writeKey\":\""+String(Amb_Key);     // (項目名)writeKey,(値)ライトキー
    S += "\",\"d1\":\"" + String(temp,2);       // (項目名)d1,(値)温度
    S += "\",\"d2\":\"" + String(hum,2) + "\"}"; // (項目名)d2,(値)湿度
    HTTPClient http;                            // HTTPリクエスト用インスタンス
    http.setConnectTimeout(15000);              // タイムアウトを15秒に設定する
    String url = "http://ambidata.io/api/v2/channels/"+String(Amb_Id)+"/data";
    http.begin(url);                            // HTTPリクエスト先を設定する
    http.addHeader("Content-Type","application/json"); // JSON形式を設定する
    Serial.println(url);                        // 送信URLを表示
    http.POST(S);                               // センサ値をAmbientへ送信する
    http.end();                                 // HTTP通信を終了する
    sleep();                                    // 下記のsleep関数を実行
}

void sleep(){
    delay(200);                                 // 送信待ち時間
    led_off();                                  // (WS2812)LEDの消灯
    Serial.println("Sleep...");                 // 「Sleep」をシリアル出力表示
    esp_deep_sleep(SLEEP_P);                    // Deep Sleepモードへ移行
}

/*******************************************************************************
UDP送信したときの動作例
********************************************************************************
ESP-ROM:esp32c3-api1-20210207
11:57:20.152 -> Build:Feb  7 2021
11:57:20.186 -> rst:0x5 (DSLEEP),boot:0xc (SPI_FAST_FLASH_BOOT)
11:57:20.186 -> SPIWP:0xee
11:57:20.186 -> mode:DIO, clock div:1
11:57:20.186 -> load:0x3fcd6100,len:0x420
11:57:20.186 -> load:0x403ce000,len:0x90c
11:57:20.186 -> load:0x403d0000,len:0x236c
11:57:20.186 -> SHA-256 comparison failed:
11:57:20.186 -> Calculated: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
11:57:20.186 -> Expected: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
11:57:20.186 -> Attempting to boot anyway...
11:57:20.240 -> entry 0x403ce000
11:57:20.452 -> ESP32C3 HUM ←-------------------【起動メッセージ】
11:57:23.475 -> 192.168.1.255 ←-----------------【UDP送信先】
11:57:23.533 -> humid_1,20.6, 58.1 ←------------【humid_1 温度20.6℃,湿度58.1%】
11:57:23.706 -> Sleep... ←----------------------【スリープモードへ移行】
*/