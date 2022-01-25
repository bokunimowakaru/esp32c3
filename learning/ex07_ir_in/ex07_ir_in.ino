/*******************************************************************************
Example 7: ESP32C3 Wi-Fi リモコン赤外線レシーバ
赤外線リモコン信号を受信し、受信データをWi-Fi送信します。

    GPIO 0 へIRセンサを接続

M5Stamp C3 or M5Stamp C3U + IR Unit に対応

                                          Copyright (c) 2016-2022 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define DATA_LEN_MAX 16                     // リモコンコードのデータ長(byte)
#define PIN_IR_IN 0                         // IO0 にIRセンサを接続
#define PIN_LED_RGB 2                       // IO2 に WS2812を接続(m5stamp)
// #define PIN_LED_RGB 8                    // IO8 に WS2812を接続(DevKitM)
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define PORT 1024                           // 送信のポート番号
#define DEVICE "ir_in_1,"                   // デバイス名(5文字+"_"+番号+",")

IPAddress IP_BROAD;                         // ブロードキャストIPアドレス

void setup(){                               // 起動時に一度だけ実行する関数
    ir_read_init(PIN_IR_IN);                // IRセンサの入力ポートの設定
    led_setup(PIN_LED_RGB);                 // WS2812の初期設定(ポート設定)
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32C3 eg.7 ir_in");   // タイトルをシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        led((millis()/50) % 10);            // (WS2812)LEDの点滅
        delay(50);                          // 待ち時間処理
    }
    IP_BROAD = WiFi.localIP();              // IPアドレスを取得
    IP_BROAD[3] = 255;                      // ブロードキャストアドレスに
    Serial.println(IP_BROAD);               // ブロードキャストアドレス表示
}

void loop(){
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    byte data[DATA_LEN_MAX];                // リモコン信号データ用
    int len,len8;                           // 信号長 len(bits),len8（bytes）
    byte i;

    led(0,20,0);                            // (WS2812)LEDを緑色で点灯
    len = ir_read(data, DATA_LEN_MAX, 255); // 赤外線信号を読み取る
    len8 = len / 8;                         // ビット長を8で割った値をlen8へ代入
    if(len%8) len8++;                       // 余りがあった場合に1バイトを加算
    if(len8>=2){                            // 2バイト以上の時に以下を実行
        led(40,0,0);                        // (WS2812)LEDを赤色で点灯
        udp.beginPacket(IP_BROAD, PORT);    // UDP送信先を設定
        udp.print(DEVICE);                  // デバイス名を送信
        udp.print(len);                     // 信号長を送信
        Serial.print(len);                  // 信号長をシリアル出力表示
        for(i=0;i<len8;i++){                // 信号長(バイト)の回数の繰り返し
            udp.print(   ",");              // 「,」カンマを送信
            Serial.print(",");              // 「,」カンマを表示
            udp.print(   data[i]>>4,HEX);   // dataを16進で送信(上位4ピット)
            Serial.print(data[i]>>4,HEX);   // dataを16進で表示(上位4ピット)
            udp.print(   data[i]&15,HEX);   // dataを16進で送信(下位4ピット)
            Serial.print(data[i]&15,HEX);   // dataを16進で表示(下位4ピット)
        }
        Serial.println();                   // 改行をシリアル出力表示
        udp.println();                      // 改行をUDP送信
        udp.endPacket();                    // UDP送信の終了(実際に送信する)
        delay(500);                         // 連続受信防止用の待ち時間処理
    }
}
