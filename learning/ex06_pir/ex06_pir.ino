/*******************************************************************************
Practice esp32 pir 【Wi-Fi 人感センサ子機】ディープスリープ版

人感センサ PIR Unit が人体などの動きを検知すると、ディープ・スリープから復帰し、
UDPブロードキャストでセンサ値を送信します。

M5Stamp C3/C3U + PIR Unit に対応

                                           Copyright (c) 2016-2022 Wataru KUNINO
********************************************************************************
「超特急Web接続! ESPマイコン・プログラム全集」 P.75の回路を使用する場合は、
以下留意してください。
・人感センサの出力インピーダンスが低い。FETかトランジスタで反転させてから IO 0 に入力する
・検知時にHighとなるときはPIR_XORを0に、LowになるときはPIR_XORを1に設定する
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

#define PIN_PIR 0                               // IO0にセンサ(人感/ドア)を接続
#define PIN_LED_RGB 2                           // IO2 に WS2812を接続(m5stamp)
// #define PIN_LED_RGB 8                        // IO8 に WS2812を接続(DevKitM)
#define SSID "1234ABCD"                         // 無線LANアクセスポイントSSID
#define PASS "password"                         // パスワード
#define PORT 1024                               // 受信ポート番号
#define DEVICE "pir_s_1,"                       // 人感センサ時デバイス名
// #define DEVICE "rd_sw_1,"                    // ドアセンサ時デバイス名
#define PIR_XOR 0                               // センサ送信値の論理反転の有無

RTC_DATA_ATTR boolean PIR;                      // pir値のバックアップ保存用
boolean pir;                                    // 人感センサ値orドアセンサ状態
esp_deepsleep_gpio_wake_up_mode_t pir_wake;     // 起動用のpir値
IPAddress IP_BROAD;                             // ブロードキャストIPアドレス
int wake = (int)esp_sleep_get_wakeup_cause();   // 起動理由を変数wakeに保存

void setup(){                                   // 起動時に一度だけ実行する関数
    pinMode(PIN_PIR,INPUT);                     // センサ接続したポートを入力に
    pir = digitalRead(PIN_PIR);                 // 人感センサの状態を取得
    led_setup(PIN_LED_RGB);                     // WS2812の初期設定(ポート設定)

    Serial.begin(115200);                       // 動作確認のためのシリアル出力
    Serial.println("ESP32C3 PIR/Reed");         // 「ESP32C3 PIR/Reed」を出力
    Serial.print(" Wake = ");                   // 「wake =」をシリアル出力表示
    Serial.println(wake);                       // 起動理由noをシリアル出力表示
    if(wake != 7) sleep();                      // ボタン以外で起動時にスリープ

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
    pir = digitalRead(PIN_PIR);                 // 人感センサの最新の状態を取得
    String S = String(DEVICE);                  // 送信データ保持用の文字列変数
    S += String(int(PIR ^ PIR_XOR)) + ", ";     // 起動時PIR値を送信データに追記
    S += String(int(pir ^ PIR_XOR));            // 現在のpir値を送信データに追記
    Serial.println(S);                          // シリアル出力表示

    WiFiUDP udp;                                // UDP通信用のインスタンスを定義
    udp.beginPacket(IP_BROAD, PORT);            // UDP送信先を設定
    udp.println(S);                             // センサ値を送信
    udp.endPacket();                            // UDP送信の終了(実際に送信する)
    delay(10);                                  // 送信待ち時間

    HTTPClient http;                            // HTTPリクエスト用インスタンス
    http.setConnectTimeout(15000);              // タイムアウトを15秒に設定する
    String url;                                 // URLを格納する文字列変数を生成
    if(strlen(LINE_TOKEN) > 42){                // LINE_TOKEN設定時
        url = "https://notify-api.line.me/api/notify";  // LINEのURLを代入
        Serial.println(url);                    // 送信URLを表示
        http.begin(url);                        // HTTPリクエスト先を設定する
        http.addHeader("Content-Type","application/x-www-form-urlencoded");
        http.addHeader("Authorization","Bearer " + String(LINE_TOKEN));
        http.POST("message=センサが反応しました。(" + S.substring(8) + ")");
        http.end();                             // HTTP通信を終了する
    }
    sleep();                                    // 下記のsleep関数を実行
}

void sleep(){                                   // スリープ実行用の関数
    int i = 0;                                  // ループ用の数値変数i
    while(i<100){                               // スイッチ・ボタン解除待ち
        boolean pir_b = digitalRead(PIN_PIR);   // 現在の値をpir_bに保存
        if( pir == pir_b) i++;                  // 値に変化がない時はiに1を加算
        else{                                   // 変化があった時
            i = 0;                              // 繰り返し用変数iを0に
            pir = pir_b;                        // pir値を最新の値に更新
        }
        delay(1);                               // 待ち時間処理
    }
    PIR = !pir;                                 // sleep時の保存可能な変数に保持
    if(PIR) pir_wake = ESP_GPIO_WAKEUP_GPIO_HIGH; // 次回、IOがHighのときに起動
    else    pir_wake = ESP_GPIO_WAKEUP_GPIO_LOW;  // 次回、IOがLowのときに起動
    led_off();                                  // (WS2812)LEDの消灯
    Serial.println("Sleep...");                 // 「Sleep」をシリアル出力表示
    delay(10);                                  // 待ち時間処理
    uint64_t pin = 1ULL << PIN_PIR;             // 起動用IOポートのマスク作成
    esp_deep_sleep_enable_gpio_wakeup(pin, pir_wake); // スリープ解除設定
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

/*******************************************************************************
初期起動後、人感センサが反応したときの動作例
********************************************************************************
（初期起動・一部略）
21:10:29.970 -> ESP32C3 PIR/Reed ←--------------【起動メッセージ】
21:10:29.970 ->  Wake = 0 ←---------------------【リセット起動時0】
21:10:30.070 -> Sleep... ←----------------------【スリープモードへ移行】

（センサ反応時）
21:10:50.968 -> ESP-ROM:esp32c3-api1-20210207
21:10:50.968 -> Build:Feb  7 2021
21:10:51.090 -> rst:0x5 (DSLEEP),boot:0xc (SPI_FAST_FLASH_BOOT)
21:10:51.090 -> SPIWP:0xee
21:10:51.090 -> mode:DIO, clock div:1
21:10:51.090 -> load:0x3fcd6100,len:0x420
21:10:51.090 -> load:0x403ce000,len:0x90c
21:10:51.090 -> load:0x403d0000,len:0x236c
21:10:51.090 -> SHA-256 comparison failed:
21:10:51.090 -> Calculated: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
21:10:51.090 -> Expected: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
21:10:51.090 -> Attempting to boot anyway...
21:10:51.090 -> entry 0x403ce000
21:10:51.233 -> ESP32C3 PIR/Reed ←--------------【起動メッセージ】
21:10:51.233 ->  Wake = 7 ←---------------------【ボタン起動時7】
21:10:54.252 -> 192.168.1.255 ←-----------------【UDP送信先】
21:10:54.252 -> pir_s_1,1, 1 ←------------------【検知時に1】
21:10:54.365 -> Sleep... ←----------------------【スリープモードへ移行】
*/
