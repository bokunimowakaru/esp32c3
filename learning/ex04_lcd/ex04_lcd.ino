/*******************************************************************************
Example 37(=32+5): ESP32 Wi-Fi LCD UDP版
各種IoTセンサが送信したデータを液晶ディスプレイ（LCD）へ表示します。

    ESP32 I2Cポート:
                        I2C SDAポート GPIO 21
                        I2C SCLポート GPIO 22
                        
                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                               // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                            // UDP通信を行うライブラリ
#define PIN_LED_RGB 2                           // IO2 に WS2812を接続(m5stamp)
// #define PIN_LED_RGB 8                        // IO8 に WS2812を接続(DevKitM)
#define SSID "1234ABCD"                         // 無線LANアクセスポイントのSSID
#define PASS "password"                         // パスワード
#define PORT 1024                               // 受信ポート番号

WiFiUDP udp;                                    // UDP通信用のインスタンスを定義
IPAddress IP_BROAD;                             // ブロードキャストIPアドレス

void setup(){                                   // 起動時に一度だけ実行する関数
    led_setup(PIN_LED_RGB);                     // WS2812の初期設定(ポートを設定)
    lcdSetup(8,2,1,0);                          // LCD初期化(X=8,Y=2,SDA=1,SCL=0)
    Serial.begin(115200);                       // 動作確認のためのシリアル出力開始
    Serial.println("ESP32C3 LCD");              // 「ESP32C3 LCD」をシリアル出力

    WiFi.mode(WIFI_STA);                        // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                      // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){       // 接続に成功するまで待つ
        led((millis()/50) % 10);                // WS2812の点滅
        delay(50);                              // 待ち時間処理
    }
    led(0,20,0);                                // (WS2812)LEDを緑色で点灯
    lcdPrintIp(WiFi.localIP());                 // 本機のIPアドレスを液晶に表示
    Serial.println(WiFi.localIP());             // 本機のIPアドレスをシリアル出力
    udp.begin(PORT);                            // UDP通信御開始
}

void loop(){                                    // 繰り返し実行する関数
    char c;                                     // 文字変数cを定義
    char lcd[49];                               // 表示用変数を定義(49バイト48文字)
    int len;                                    // 文字列長を示す整数型変数を定義
    
    memset(lcd, 0, 49);                         // 文字列変数lcdの初期化(49バイト)
    len = udp.parsePacket();                    // 受信パケット長を変数lenに代入
    if(len==0)return;                           // 未受信のときはloop()の先頭に戻る
    led(20,0,0);                                // (WS2812)LEDを赤色に変更
    udp.read(lcd, 48);                          // 受信データを文字列変数lcdへ代入
    Serial.print(lcd);                          // シリアルへ出力する
    lcdPrint(lcd);                              // 液晶に表示する
    led(0,20,0);                                // (WS2812)LEDを緑色に戻す
}
