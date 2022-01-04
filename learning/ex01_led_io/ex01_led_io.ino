/*******************************************************************************
Example 1: ESP32C3 Wi-Fi コンシェルジェ 照明担当
HTTPによるWebサーバ機能搭載 Wi-FiコンシェルジェがLEDを制御します。

                                          Copyright (c) 2021-2022 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WebServer.h>                      // HTTPサーバ用ライブラリ

#define PIN_LED 0                           // IO 0 に (通常の)LED を接続
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
    }else{
        digitalWrite(PIN_LED, LOW);         // LEDを消灯
    }
}

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED, OUTPUT);               // (通常の)LED用のIOポートを出力に
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32C3 LED HTTP");     // 「LED HTTP」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        digitalWrite(PIN_LED,millis()/100%2); // LEDの点滅
        delay(50);                          // 待ち時間処理
    }
    server.on("/", handleRoot);             // HTTP接続時のコールバック先を設定
    server.begin();                         // Web サーバを起動する
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
}

void loop(){                                // 繰り返し実行する関数
    server.handleClient();                  // クライアントからWebサーバ呼び出し
}
