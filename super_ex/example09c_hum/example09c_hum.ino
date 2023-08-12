/*******************************************************************************
Example 9c: (IoTセンサ) Wi-Fi 温湿度計 TI製 HDC1000  [Ambient対応版]
温湿度を送信するIoTセンサのクラウドサービスAmbientへの送信機能つき版です。

                                          Copyright (c) 2016-2019 Wataru KUNINO
********************************************************************************
本サンプルで使用するクラウドサービスAmbient用の設定確認ページ
http://ambidata.io/ch/channels.html
*******************************************************************************/

#include <WiFi.h>                           // WiFi用ライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include "Ambient.h"                        // Ambient用のライブラリの組み込み
#define PIN_LED 6                           // IO 6(5番ピン)にLEDを接続する
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define AmbientChannelId 100                // チャネルID(整数)
#define AmbientWriteKey "0123456789abcdef"  // ライトキー(16桁の16進数)
#define SENDTO "192.168.X.X"                // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
#define SLEEP_P 29*60*1000000ul             // スリープ時間 29分(uint32_t)
#define DEVICE "humid_1,"                   // デバイス名(5文字+"_"+番号+",")
void sleep();
Ambient ambient;
WiFiClient client;
IPAddress IP;                               // ブロードキャストIPアドレス

void setup(){                               // 起動時に一度だけ実行する関数
    int waiting=0;                          // アクセスポイント接続待ち用
    pinMode(PIN_LED,OUTPUT);                // LED用ポートを出力に
    digitalWrite(PIN_LED,HIGH);             // LEDの点灯
    hdcSetup();                             // 湿度センサの初期化
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("Example 09C HUM->Amb"); // 「Example 09」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(100);                         // 待ち時間処理
        waiting++;                          // 待ち時間カウンタを1加算する
        if(waiting%10==0)Serial.print('.'); // 進捗表示
        if(waiting > 300) sleep();          // 300回(30秒)を過ぎたらスリープ
    }
    IP = WiFi.localIP();                    // IPアドレスを取得
    Serial.println(IP);                     // 本機のIPアドレスをシリアル出力
    IP[3] = 255;                            // ブロードキャストアドレスに
    ambient.begin(AmbientChannelId, AmbientWriteKey, &client);  // Ambient開始
}

void loop(){
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    float temp,hum;                         // センサ用の浮動小数点数型変数
    char s[6];
    
    temp=getTemp();                         // 温度を取得して変数tempに代入
    hum =getHum();                          // 湿度を取得して変数humに代入
    if( temp>-100. && hum>=0.){             // 適切な値の時
        if(!String(SENDTO).compareTo("192.168.X.X")){ // 初期状態の時
            udp.beginPacket(IP, PORT);      // UDP送信先をブロードキャストに設定
        }else{
            udp.beginPacket(SENDTO, PORT);  // UDP送信先を定義済SENDTOに設定
        }
        udp.print(DEVICE);                  // デバイス名を送信
        udp.print(temp,1);                  // 変数tempの値を送信
        Serial.print(temp,2);               // シリアル出力表示
        udp.print(", ");                    // 「,」カンマを送信
        Serial.print(", ");                 // シリアル出力表示
        udp.println(hum,1);                 // 変数humの値を送信
        Serial.println(hum,2);              // シリアル出力表示
        udp.endPacket();                    // UDP送信の終了(実際に送信する)
        /* クラウドへ */
        dtostrf(temp,-5,2,s);               // 温度を文字列に変換
        ambient.set(1,s);                   // Ambient(データ1)へ温度を送信
        dtostrf(hum,-5,2,s);                // 湿度を文字列に変換
        ambient.set(2,s);                   // Ambient(データ2)へ湿度を送信
        ambient.send();                     // Ambient送信の終了(実際に送信する)
    }
    sleep();
}

void sleep(){
    digitalWrite(PIN_LED,LOW);              // LEDの消灯
    delay(200);                             // 送信待ち時間
    esp_deep_sleep(SLEEP_P);                    // Deep Sleepモードへ移行
    while(1){                               // 繰り返し処理
        delay(100);                         // 100msの待ち時間処理
    }                                       // 繰り返し中にスリープへ移行
}
