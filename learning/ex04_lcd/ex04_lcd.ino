/*******************************************************************************
Example 4: ESP32C3 Wi-Fi コンシェルジェ 掲示板担当
各種IoTセンサが送信したデータを液晶ディスプレイ LCD AQM0802 へ表示します。
HTTPによるWebサーバ機能搭載 Wi-FiコンシェルジェがLCDを制御します。

    ESP32 のI2Cポート:
        AQM0802 SDAポート GPIO 1
        AQM0802 SCLポート GPIO 0
        設定方法＝lcdSetup(表示xサイズ,表示yサイズ,SDAポート番号,SCLポート番号)

M5Stamp C3, M5Stamp C3U に対応
                        
                                          Copyright (c) 2016-2022 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                               // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                            // UDP通信を行うライブラリ
#include <WebServer.h>                          // HTTPサーバ用ライブラリ

#define PIN_LED_RGB 2                           // IO2 に WS2812を接続(m5stamp)
// #define PIN_LED_RGB 8                        // IO8 に WS2812を接続(DevKitM)
#define SSID "1234ABCD"                         // 無線LANアクセスポイントSSID
#define PASS "password"                         // パスワード
#define PORT 1024                               // 受信ポート番号

WebServer server(80);                           // Webサーバ(ポート80=HTTP)定義

void handleRoot(){
    String rx, tx;                              // 受信用,送信用文字列
    led(20,0,0);                                // (WS2812)LEDを赤色に変更
    if(server.hasArg("TEXT")){                  // クエリTEXTが含まれていた時
        rx = server.arg("TEXT");                // クエリ値を文字変数rxへ代入
    }
    tx = getHtml(rx);                           // HTMLコンテンツを取得
    server.send(200, "text/html", tx);          // HTMLコンテンツを送信
    Serial.println(rx);                         // シリアルへ出力する
    lcdPrint(rx);
    led(0,20,0);                                // (WS2812)LEDを緑色に戻す
}

WiFiUDP udp;                                    // UDP通信用のインスタンスを定義

void setup(){                                   // 起動時に一度だけ実行する関数
    led_setup(PIN_LED_RGB);                     // WS2812の初期設定(ポート設定)
    lcdSetup(8,2,1,0);                          // LCD初期(X=8,Y=2,SDA=1,SCL=0)
    Serial.begin(115200);                       // 動作確認のためのシリアル出力
    Serial.println("ESP32C3 LCD");              // 「ESP32C3 LCD」をシリアル出力

    WiFi.mode(WIFI_STA);                        // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                      // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){       // 接続に成功するまで待つ
        led((millis()/50) % 10);                // WS2812の点滅
        delay(50);                              // 待ち時間処理
    }
    led(0,20,0);                                // (WS2812)LEDを緑色で点灯
    lcdPrintIp(WiFi.localIP());                 // 本機のIPアドレスを液晶に表示
    server.on("/", handleRoot);                 // HTTP接続時コールバック先設定
    server.begin();                             // Web サーバを起動する
    Serial.println(WiFi.localIP());             // 本機IPアドレスをシリアル出力
    udp.begin(PORT);                            // UDP通信御開始
}

void loop(){                                    // 繰り返し実行する関数
    server.handleClient();                      // クライアントからWebサーバ呼出
    char lcd[49];                               // 表示用変数(49バイト48文字)
    memset(lcd, 0, 49);                         // 文字列変数lcd初期化(49バイト)
    int len = udp.parsePacket();                // 受信パケット長を変数lenに代入
    if(len==0)return;                           // 未受信のときはloop()の先頭に
    led(20,0,0);                                // (WS2812)LEDを赤色に変更
    udp.read(lcd, 48);                          // 受信データを文字列変数lcdへ
    udp.flush();                                // 受信できなかったデータを破棄
    Serial.print(lcd);                          // シリアルへ出力する
    lcdPrint(lcd);                              // 液晶に表示する
    led(0,20,0);                                // (WS2812)LEDを緑色に戻す
}

/*******************************************************************************
インターネット・ブラウザからLCDを制御したときの動作例
********************************************************************************
12:13:32.827 -> ESP-ROM:esp32c3-api1-20210207
12:13:32.860 -> Build:Feb  7 2021
12:13:32.860 -> rst:0x1 (POWERON),boot:0xc (SPI_FAST_FLASH_BOOT)
12:13:32.860 -> SPIWP:0xee
12:13:32.860 -> mode:DIO, clock div:1
12:13:32.860 -> load:0x3fcd6100,len:0x420
12:13:32.860 -> load:0x403ce000,len:0x90c
12:13:32.860 -> load:0x403d0000,len:0x236c
12:13:32.860 -> SHA-256 comparison failed:
12:13:32.860 -> Calculated: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
12:13:32.860 -> Expected: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
12:13:32.860 -> Attempting to boot anyway...
12:13:32.921 -> entry 0x403ce000
12:13:33.391 -> ESP32C3 LCD ←-------------------【起動メッセージ】
12:13:36.508 -> 192.168.1.7 ←-------------------【本機のIPアドレス】
12:15:34.421 -> ｴﾚｷｼﾞｬｯｸIoT CQpb ←---------------【LCDにメッセージを表示】

********************************************************************************
ex02_sw や ex03_lum 、 ex05_hum からUDPを受信したときの表示例
********************************************************************************
12:25:21.657 -> ESP-ROM:esp32c3-api1-20210207
12:25:21.690 -> Build:Feb  7 2021
12:25:21.690 -> rst:0x1 (POWERON),boot:0xc (SPI_FAST_FLASH_BOOT)
12:25:21.690 -> SPIWP:0xee
12:25:21.690 -> mode:DIO, clock div:1
12:25:21.690 -> load:0x3fcd6100,len:0x420
12:25:21.690 -> load:0x403ce000,len:0x90c
12:25:21.690 -> load:0x403d0000,len:0x236c
12:25:21.690 -> SHA-256 comparison failed:
12:25:21.690 -> Calculated: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
12:25:21.690 -> Expected: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
12:25:21.690 -> Attempting to boot anyway...
12:25:21.690 -> entry 0x403ce000
12:25:22.187 -> ESP32C3 LCD ←-------------------【起動メッセージ】
12:25:26.333 -> 192.168.1.7 ←-------------------【本機のIPアドレス】
12:25:38.968 -> Ping ←--------------------------【ex02_sw のボタンが押された】
12:26:20.355 -> illum_1,1300 ←------------------【ex03_lum 照度1300lx】
12:26:43.682 -> humid_1,20.6, 58.1 ←------------【ex05_hum 温度20.6℃,湿度58%】
*/
