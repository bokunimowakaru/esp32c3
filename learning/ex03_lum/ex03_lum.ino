/*******************************************************************************
Example 3: ESP32C3 (IoTセンサ) Wi-Fi 照度計
照度センサ NJL7502L から取得した照度値を送信するIoTセンサです。

                                          Copyright (c) 2016-2022 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                               // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                            // UDP通信を行うライブラリ
#include <HTTPClient.h>                         // HTTPクライアント用ライブラリ
#include "esp_sleep.h"                          // ESP32用Deep Sleep ライブラリ

#define PIN_LED_RGB 2                           // IO2 に WS2812を接続(m5stamp)
// #define PIN_LED_RGB 8                        // IO8 に WS2812を接続(DevKitM)
#define PIN_EN 0                                // GPIO 0をセンサの電源に
#define PIN_AIN 1                               // GPIO 1を照度センサの信号入力に
#define SSID "1234ABCD"                         // 無線LANアクセスポイントのSSID
#define PASS "password"                         // パスワード
#define PORT 1024                               // 送信のポート番号
#define SLEEP_P 30*1000000ul                    // スリープ時間 30秒(uint32_t)
#define DEVICE "illum_1,"                       // デバイス名(5文字+"_"+番号+",")

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
    pinMode(PIN_AIN,ANALOG);                    // アナログ入力の設定
    pinMode(PIN_EN,OUTPUT);                     // センサ用の電源を出力に
    Serial.begin(115200);                       // 動作確認のためのシリアル出力開始
    Serial.println("ESP32C3 LUM");              // 「ESP32C3 LUM」をシリアル出力

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

void loop(){                                    // 繰り返し実行する関数
    digitalWrite(PIN_EN,HIGH);                  // センサ用の電源をONに
    delay(100);                                 // 起動待ち時間
    float lux = analogRead(PIN_AIN) * 100./ 33.; // 照度(lux)へ変換
    digitalWrite(PIN_EN,LOW);                   // センサ用の電源をOFFに

    String S = String(DEVICE) + String(lux,0);  // 送信データSにデバイス名を代入
    Serial.println(S);                          // 送信データSをシリアル出力表示
    WiFiUDP udp;                                // UDP通信用のインスタンスを定義
    udp.beginPacket(IP_BROAD, PORT);            // UDP送信先を設定
    udp.println(S);                             // 送信データSをUDP送信
    udp.endPacket();                            // UDP送信の終了(実際に送信する)
    if(strcmp(Amb_Id,"00000") == 0) sleep();    // Ambient未設定時にsleepを実行

    S = "{\"writeKey\":\""+String(Amb_Key);     // (項目名)writeKey,(値)ライトキー
    S += "\",\"d1\":\"" + String(lux) + "\"}";  // (項目名)d1,(値)照度
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

void sleep(){                                   // スリープ実行用の関数    Serial.print(" SW   = ");                   // 「SW = 」をシリアル出力表示
    delay(200);                                 // 送信待ち時間
    led_off();                                  // (WS2812)LEDの消灯
    Serial.println("Sleep...");                 // 「Sleep」をシリアル出力表示
    esp_deep_sleep(SLEEP_P);                    // Deep Sleepモードへ移行
}

/*******************************************************************************
照度値を取得し、UDP送信したときの動作例
********************************************************************************
12:06:05.957 -> ESP-ROM:esp32c3-api1-20210207
12:06:05.957 -> Build:Feb  7 2021
12:06:05.957 -> rst:0x5 (DSLEEP),boot:0xc (SPI_FAST_FLASH_BOOT)
12:06:05.957 -> SPIWP:0xee
12:06:05.957 -> mode:DIO, clock div:1
12:06:05.957 -> load:0x3fcd6100,len:0x420
12:06:05.990 -> load:0x403ce000,len:0x90c
12:06:05.990 -> load:0x403d0000,len:0x236c
12:06:05.990 -> SHA-256 comparison failed:
12:06:05.990 -> Calculated: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
12:06:05.990 -> Expected: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
12:06:05.990 -> Attempting to boot anyway...
12:06:05.990 -> entry 0x403ce000
12:06:06.224 -> ESP32C3 LUM ←-------------------【起動メッセージ】
12:06:09.243 -> 192.168.1.255 ←-----------------【UDP送信先】
12:06:09.342 -> illum_1,1300 ←------------------【illum_1 照度1300lx】
12:06:09.541 -> Sleep... ←----------------------【スリープモードへ移行】
*/