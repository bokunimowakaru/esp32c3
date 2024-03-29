/*******************************************************************************
Example 9: (IoTセンサ) Wi-Fi 温湿度計 SILICON LABS製 Si7021 版 【ESP32-C3版】
デジタルI2Cインタフェース搭載センサから取得した温湿度を送信するIoTセンサです。

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.X.X"                // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
#define SLEEP_P 29*60*1000000ul             // スリープ時間 29分(uint32_t)
#define DEVICE "humid_1,"                   // デバイス名(5文字+"_"+番号+",")
void sleep();
IPAddress IP;                               // ブロードキャストIPアドレス

void setup(){                               // 起動時に一度だけ実行する関数
    int waiting=0;                          // アクセスポイント接続待ち用
    si7021Setup();                          // 湿度センサの初期化
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("Example 09 HUM");       // 「Example 09」をシリアル出力表示
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
}

void loop(){
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    float temp,hum;                         // センサ用の浮動小数点数型変数
    
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
    }
    sleep();
}

void sleep(){
    delay(200);                             // 送信待ち時間
    esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
    while(1){                               // 繰り返し処理
        delay(100);                         // 100msの待ち時間処理
    }                                       // 繰り返し中にスリープへ移行
}
