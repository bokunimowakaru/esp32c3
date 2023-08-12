/*******************************************************************************
Example 8: (IoTセンサ) Wi-Fi ドア開閉モニタ リードスイッチ
                                               [OFF検出(L->H)専用]【ESP32-C3版】
                                               
ドアや窓が開いたとき（または閉まったとき）に開閉状態を送信するIoTセンサです。

                                          Copyright (c) 2016-2023 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define PIN_SW 10                           // IO 10(10番ピン) にスイッチを接続
#define PIN_LED 6                           // IO 6(5番ピン)にLEDを接続する
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.X.X"                // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
#define SLEEP_P 590*1000000ul               // スリープ時間 590秒(約10分)
#define DEVICE "rd_sw_1,"                   // デバイス名(5文字+"_"+番号+",")

void sleep();

RTC_DATA_ATTR boolean mem = 0;              // reed値のバックアップ保存用
IPAddress IP;                               // ブロードキャストIPアドレス
int reed;                                   // リードスイッチの状態用

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_SW,INPUT_PULLUP);           // スイッチを接続したポートを入力に
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("Example 08 REED SW");   // 「Example 08」をシリアル出力表示
    reed=digitalRead(PIN_SW);               // スイッチの状態を取得
/*
    Serial.print(mem);                      // メモリ内の状態を出力表示
    Serial.print(", ");                     // 「,」カンマと「␣」を出力表示
    Serial.println(reed);                   // 起動直後のスイッチ状態を出力表示
*/

    if(reed == mem) sleep();                // センサに変化が無かった場合は終了
    mem = reed;                             // RTCメモリへ保存
    if(reed == 0) sleep();                  // High -> Low時は終了
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    int waiting=0;                          // アクセスポイント接続待ち用
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(100);                         // 待ち時間処理
        waiting++;                          // 待ち時間カウンタを1加算する
        digitalWrite(PIN_LED,waiting%2);    // LED(EN信号)の点滅
        if(waiting%10==0)Serial.print('.'); // 進捗表示
        if(waiting > 300) sleep();          // 300回(30秒)を過ぎたらスリープ
    }
    IP = WiFi.localIP();                    // IPアドレスを取得
    Serial.println(IP);                     // 本機のIPアドレスをシリアル出力
    IP[3] = 255;                            // ブロードキャストアドレスに
    Serial.print(reed);                     // 起動直後のスイッチ状態を出力表示
    Serial.print(", ");                     // 「,」カンマと「␣」を出力表示
}

void loop(){
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    
    if(!String(SENDTO).compareTo("192.168.X.X")){ // 初期状態の時
        udp.beginPacket(IP, PORT);          // UDP送信先をブロードキャストに設定
    }else{
        udp.beginPacket(SENDTO, PORT);      // UDP送信先を定義済SENDTOに設定
    }
    udp.print(DEVICE);                      // デバイス名を送信
    udp.print(reed);                        // 起動直後のスイッチ状態を送信
    udp.print(", ");                        // 「,」カンマと「␣」を送信
    reed=digitalRead(PIN_SW);               // スイッチの状態を取得
    udp.println(reed);                      // 現在のスイッチの状態を送信
    Serial.println(reed);                   // シリアル出力表示
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    sleep();
}

void sleep(){
    Serial.println("Sleep");                // シリアル出力表示
    digitalWrite(PIN_LED,LOW);              // LEDの消灯
    pinMode(PIN_SW,INPUT);                  // スイッチを接続したポートを入力に
    delay(200);                             // 送信待ち時間
    esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
    while(1){                               // 繰り返し処理
        delay(100);                         // 100msの待ち時間処理
    }                                       // 繰り返し中にスリープへ移行
}
