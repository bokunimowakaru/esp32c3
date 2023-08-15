/*******************************************************************************
Example 5: Wi-Fi LCD UDP版 【ESP32-C3版】
各種IoTセンサが送信したデータを液晶ディスプレイ（LCD）へ表示します。

                                          Copyright (c) 2016-2023 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define PORT 1024                           // 受信ポート番号
WiFiUDP udp;                                // UDP通信用のインスタンスを定義

void setup(){                               // 起動時に一度だけ実行する関数
    lcdSetup();                             // 液晶の初期化
    lcdPrint("Example 05 LCD");             // 「Example 05」をLCDに表示する
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
    }
    lcdPrintIp(WiFi.localIP());             // 本機のIPアドレスを液晶に表示
    udp.begin(PORT);                        // UDP通信御開始
}

void loop(){                                // 繰り返し実行する関数
    char c;                                 // 文字変数cを定義
    char lcd[49];                           // 表示用変数を定義(49バイト48文字)
    int len;                                // 文字列長を示す整数型変数を定義
    
    memset(lcd, 0, 49);                     // 文字列変数lcdの初期化(49バイト)
    len = udp.parsePacket();                // 受信パケット長を変数lenに代入
    if(len==0)return;                       // 未受信のときはloop()の先頭に戻る
    udp.read(lcd, 48);                      // 受信データを文字列変数lcdへ代入
    lcdPrint(lcd);                          // 液晶に表示する
}
