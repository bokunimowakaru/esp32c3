/*******************************************************************************
Example 48 (=32+16): ESP32 Wi-Fi コンシェルジェ 照明担当 (キャンドルLED制御)
HTTPによるWebサーバ機能搭載 Wi-FiコンシェルジェがキャンドルLEDを制御します。
[ESP8266WebServer対応版]
                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WebServer.h>
#define PIN_LED 8                           // GPIO 8にWS2812 LEDを接続
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード

WebServer server(80);                       // Webサーバ(ポート80=HTTP)定義
int target=0;                               // LED設定値(0は消灯)

void handleRoot(){
    char html[2048];                        // Web表示用コンテンツ格納用変数
    Serial.println("Connected");            // 接続表示
    
    if(server.hasArg("L")){                 // 引数Lが含まれていた時
        String s=server.arg("L");           // 引数Lの値を取得し文字変数sへ代入
        target=s.toInt();                   // 文字変数sから数値を取得しtargetへ
    }
    getHtml(html,target);                   // HTMLコンテンツを取得
    server.send(200, "text/html", html);    // HTMLコンテンツを送信
    Serial.println("Disconnected");         // 切断表示
    
    if(target==0) led(0);                   // 消灯
    if(target==1) led(10);                  // 点灯
    if(target<=0 && target>=-10) led(-target); // 輝度変更
}

void setup(){                               // 起動時に一度だけ実行する関数
    led_setup(PIN_LED);                     // LEDを接続したポートを出力に
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32 eg.16 LED HTTP"); // 「Example 16」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    morse(20,50,"HELLO");                   // モールス信号(輝度20,速度50,HELLO)
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        Serial.print('.');                  // 進捗表示
        digitalWrite(PIN_LED,!digitalRead(PIN_LED));    // LEDの点滅
        delay(500);                         // 待ち時間処理
    }
    morseIp0(10,50,WiFi.localIP());         // IPアドレス終値をモールス信号出力
    server.on("/", handleRoot);             // HTTP接続時のコールバック先を設定
    server.begin();                         // Web サーバを起動する
    Serial.println("\nStarted");            // 起動したことをシリアル出力表示
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
}

void loop(){                                // 繰り返し実行する関数
    server.handleClient();                  // クライアントからWebサーバ呼び出し
    delay(200);
    if(target>1 && target<=10){             // 1よりも大きく10以下のとき
        int val = random(0,target);
        led(val,val,0);
    }                                       // LEDの輝度を乱数値23～1023に設定
}
