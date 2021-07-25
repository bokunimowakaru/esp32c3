/*******************************************************************************
Practice esp32 14 pir 【Wi-Fi 人感センサ子機】ディープスリープ版

◆◆PIR値の論理反転中
◆◆スリープ動作の調査中

                                           Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/
#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include "esp_sleep.h"                      // ESP32用Deep Sleep ライブラリ
#define PIN_LED 8                           // IO 8にLEDを接続
#define PIN_SW 6                            // IO 6にスイッチを接続
#define PIN_PIR 19                          // IO 14にスイッチを接続
#define PIN_PIR_GPIO_NUM GPIO_NUM_19        // GPIO 19をスリープ解除信号へ設定
#define BUTTON_PIN_BITMASK 0x000080000      // 2^(PIN_PIR) in hex
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define PORT 1024                           // 受信ポート番号
#define SLEEP_P 3550ul*1000000ul            // スリープ時間 3550秒(約60分)
#define DEVICE "pir_s_1,"                   // デバイス名(5文字+"_"+番号+",")
boolean pir;                                // 人感センサ値
IPAddress IP;                               // ブロードキャストIP保存用

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    pinMode(7,OUTPUT);
	digitalWrite(7,1);
    pinMode(PIN_PIR,INPUT_PULLUP);          // センサを接続したポートを入力に
    pinMode(PIN_SW,INPUT);                  // センサを接続したポートを入力に
    pir=!digitalRead(PIN_PIR);       //反転中        // 人感センサの状態を取得
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32 Example 06 PIR"); // 「Example 06 PIR」をシリアル出力
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        digitalWrite(PIN_LED,!digitalRead(PIN_LED));          // LEDの消灯
        Serial.print('.');                  // 進捗表示
        delay(500);                         // 待ち時間
        if(millis() > 30000) sleep();       // 30000ms(30秒)を過ぎたらスリープ
    }
    IP = WiFi.localIP();                    // IPアドレスを取得
    Serial.println(IP);
    IP[3] = 255;                            // ブロードキャストアドレスに
    boolean sw=digitalRead(PIN_SW);
    while(sw){
		sw=digitalRead(PIN_SW);
		Serial.print(sw);
		Serial.print(", ");
		pir=!digitalRead(PIN_PIR);	//反転中
		digitalWrite(PIN_LED,!pir);
		Serial.println(pir);
    	delay(1000);
	}
}

void loop(){                                // 繰り返し実行する関数
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    udp.beginPacket(IP, PORT);              // UDP送信先を設定
    udp.print(DEVICE);                      // デバイス名を送信
    udp.print(pir);                         // 起動直後のセンサ状態を送信
    udp.print(", ");                        // 「,」カンマと「␣」を送信
    pir=!digitalRead(PIN_PIR);      //反転中         // 人感センサの状態を取得
    udp.println(pir);                       // 現在のセンサの状態を送信
    Serial.println(pir);                    // シリアル出力表示
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    delay(10);                              // 送信待ち時間
    // sleep();                                // sleepを実行
    while(pir==!digitalRead(PIN_PIR)) delay(100);
}

void sleep(){
	// https://docs.espressif.com/projects/esp-idf/en/v4.3-beta1/esp32c3/api-reference/system/sleep_modes.html
	
    delay(200);                             // 送信待ち時間
    esp_sleep_enable_gpio_wakeup();
    // esp_sleep_enable_ext0_wakeup();  // 1=High,0=Low
                                            // リードスイッチの状態が変化すると
                                            // スリープを解除するように設定
	esp_light_sleep_start();
    // esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
    delay(5000);
    return;
}

/*

BS612 AS612と同じ
1.SENS		感度設定 0.12V 感度2（０～３１）
2.OEN		2.367
3.VSS
4.VDD
5.REL		出力
6.ONTIME	ホールド時間設定 0.48V 30秒程度

*/