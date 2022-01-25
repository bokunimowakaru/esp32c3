/*******************************************************************************
Example 1: ESP32C3 Wi-Fi コンシェルジェ 照明担当
HTTPによるWebサーバ機能搭載 Wi-FiコンシェルジェがLEDを制御します。

M5Stamp C3, M5Stamp C3U に対応

                                          Copyright (c) 2021-2022 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WebServer.h>                      // HTTPサーバ用ライブラリ

#define PIN_LED 0                           // IO 0 に (通常の)LED を接続
#define PIN_LED_RGB 2                       // IO 2 に WS2812 を接続(m5stamp用)
// #define PIN_LED_RGB 8                    // IO 8 に WS2812 を接続(DevKitM用)
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード

WebServer server(80);                       // Webサーバ(ポート80=HTTP)定義
int led_stat = 0;                           // LED状態用の数値変数led_statを定義

void handleRoot(){
    String rx, tx;                          // 受信用,送信用文字列

    if(server.hasArg("L")){                 // 引数Lが含まれていた時
        rx = server.arg("L");               // 引数Lの値を取得し文字変数rxへ代入
        led_stat = rx.toInt();              // 変数sから数値を取得しled_statへ
    }
    tx = getHtml(led_stat);                 // HTMLコンテンツを取得
    server.send(200, "text/html", tx);      // HTMLコンテンツを送信

    Serial.println(led_stat);               // LED状態led_stat値を表示
    if(abs(led_stat) >= 1){                 // led_statの絶対値が1以上の時
        digitalWrite(PIN_LED, HIGH);        // LEDを点灯
        led(20);                            // WS2812を点灯(輝度20)
    }else{
        digitalWrite(PIN_LED, LOW);         // LEDを消灯
        led(0);                             // WS2812を消灯
    }
}

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED, OUTPUT);               // (通常の)LED用のIOポートを出力に
    led_setup(PIN_LED_RGB);                 // WS2812の初期設定(ポートを設定)
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32C3 LED HTTP");     // 「LED HTTP」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    morse(PIN_LED,50,"HELLO");              // モールス信号(ピン,速度50,HELLO)
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        digitalWrite(PIN_LED,millis()/100%2); // LEDの点滅
        led((millis()/50) % 10);            // WS2812の点滅
        delay(50);                          // 待ち時間処理
    }
    morseIp0(PIN_LED,50,WiFi.localIP());    // IPアドレス終値をモールス信号出力
    server.on("/", handleRoot);             // HTTP接続時のコールバック先を設定
    server.begin();                         // Web サーバを起動する
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
}

void loop(){                                // 繰り返し実行する関数
    server.handleClient();                  // クライアントからWebサーバ呼び出し
}

/*******************************************************************************
インターネット・ブラウザからLEDを制御したときの動作例
********************************************************************************
11:46:44.116 -> ESP-ROM:esp32c3-api1-20210207
11:46:44.149 -> Build:Feb  7 2021
11:46:44.149 -> rst:0x1 (POWERON),boot:0xc (SPI_FAST_FLASH_BOOT)
11:46:44.149 -> SPIWP:0xee
11:46:44.149 -> mode:DIO, clock div:1
11:46:44.149 -> load:0x3fcd6100,len:0x420
11:46:44.149 -> load:0x403ce000,len:0x90c
11:46:44.149 -> load:0x403d0000,len:0x236c
11:46:44.149 -> SHA-256 comparison failed:
11:46:44.149 -> Calculated: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
11:46:44.202 -> Expected: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
11:46:44.202 -> Attempting to boot anyway...
11:46:44.202 -> entry 0x403ce000
11:46:44.381 -> ESP32C3 LED HTTP ←--------------【起動メッセージ】
11:46:44.513 -> .... . ._.. ._.. ___  ←---------【モールス信号"HELLO"】
11:46:47.232 -> Done Morse
11:46:47.464 -> ._._._ __... ←------------------【モールス信号".7"(IPの末尾)】
11:46:49.387 -> Done Morse
11:46:49.387 -> 192.168.1.7 ←-------------------【本機のIPアドレス】
11:46:55.725 -> 1 ←-----------------------------【LED ONを実行】
11:47:00.006 -> 0 ←-----------------------------【LED OFFを実行】
11:47:09.237 -> 1 ←-----------------------------【LED ONを実行】
11:47:12.103 -> 0 ←-----------------------------【LED OFFを実行】
*/