/*******************************************************************************
Example 1: Wi-Fi コンシェルジェ 照明担当 for ESP32 / ATOM / ATOM Lite
・HTTPによるWebサーバ機能搭載 Wi-FiコンシェルジェがLEDを制御します。

                                          Copyright (c) 2021-2024 Wataru KUNINO
*******************************************************************************
【参考文献】
Arduino IDE 開発環境イントール方法：
https://docs.m5stack.com/en/quick_start/atom/arduino

ATOM Lite Arduino Library API 情報(本サンプルでは使用しない)：
https://docs.m5stack.com/en/api/atom/system

【引用コード】
https://github.com/bokunimowakaru/m5/tree/master/atom/ex01_led_30
https://github.com/bokunimowakaru/esp/tree/master/2_example/example16_led
https://github.com/bokunimowakaru/esp/tree/master/2_example/example48_led
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WebServer.h>                      // HTTPサーバ用ライブラリ

#define PIN_LED_RGB 32                      // RGB LED 30個 Grove互換ポート
#define PIN_BTN 39                          // G39 に 操作ボタン
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード

WebServer server(80);                       // Webサーバ(ポート80=HTTP)定義
int led_stat = 0;                           // LED状態用の変数led_statを定義

void handleRoot(){
    String rx, tx;                          // 受信用,送信用文字列
    int r=-1,g=-1,b=-1;
    int single=0,notify=0;

    for(int i=0; i<server.args(); i++){
        String query = server.argName(i);
        Serial.print("query "+query+" = ");
        if(!query.compareTo("L")){  // 引数Lが含まれていた時
            rx = server.arg(i);               // 引数Lの値を取得し変数rxへ代入
            led_stat = rx.toInt();              // 変数sから数値を取得しled_statへ
            Serial.printf("%d\n",led_stat);
        }
        if(!query.compareTo("R")){
            r = server.arg(i).toInt();
            Serial.printf("%d\n",r);
            if( r < 0 || r > 100) r=-1;
        }
        if(!query.compareTo("G")){
            g = server.arg(i).toInt();
            Serial.printf("%d\n",g);
            if( g < 0 || g > 100) g=-1;
        }
        if(!query.compareTo("B")){
            b = server.arg(i).toInt();
            Serial.printf("%d\n",b);
            if( b < 0 || b > 100) b=-1;
        }
        if(!query.compareTo("single")){
            single=server.arg(i).toInt();
            Serial.printf("%d\n",single);
            if( single < 0 || single > 1) single=0;
        }
        if(!query.compareTo("notify")){
            notify=server.arg(i).toInt();
            Serial.printf("%d\n",notify);
            if( notify < 0 ) notify=0;
        }
    }
    if(!single && !notify){
        if( r>=0 && g>=0 && b>=0 ){
            led_stat = 1;
            led(r,g,b);
        }else if(abs(led_stat) >= 1){       // led_statの絶対値が1以上の時
            if(led_stat > 20 && led_stat <= 100){
                led(led_stat);              // RGB LEDを点灯
            }else if(led_stat < 0 && led_stat >= -30){
                led_bar(-led_stat);
            }else{
                led(20);                    // RGB LEDを点灯(輝度20)
            }
        }else{
            led(0);                         // RGB LEDを消灯
        }
    }else if(single){
        if( r>=0 && g>=0 && b>=0 ){
            led_single(r,g,b);
        }else if(led_stat >= 1){            // led_statの絶対値が1以上の時
            if(led_stat > 20 && led_stat <= 100){
                led_single(led_stat);       // RGB LEDを点灯
            }else{
                led_single(20);             // RGB LEDを点灯(輝度20)
            }
        }else{
            led_single(0);                  // RGB LEDを消灯
        }
        led_stat = 1;
    }else if(notify){
        led_notify(notify);
    }

    tx = getHtml(led_stat);                 // HTMLコンテンツを取得
    server.send(200, "text/html", tx);      // HTMLコンテンツを送信
}

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_BTN,INPUT_PULLUP);              // ボタン入力の設定
    led_setup(PIN_LED_RGB);                 // RGB LED の初期設定(ポートを設定)
    Serial.begin(115200);                   // 動作確認用シリアル出力開始
    Serial.println("M5 LED HTTP");          // 「LED HTTP」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        led((millis()/50) % 10);            // RGB LEDの点滅
        delay(50);                          // 待ち時間処理
    }
    morseIp0(-1,100,WiFi.localIP());         // IPアドレス終値をモールス信号出力
    led_num(((uint32_t)(WiFi.localIP()))>>24);
    server.on("/", handleRoot);             // HTTP接続時のコールバック先を設定
    server.begin();                         // Web サーバを起動する
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
    delay(2000);                            // IPアドレスを認知させる期間
    led(20);                                // RGB LEDを点灯(輝度20)
}

void loop(){                                // 繰り返し実行する関数
    if(!digitalRead(PIN_BTN)){
        led_notify(1);
        led_stat = !led_stat;
        if(led_stat){                       // led_statの絶対値が1以上の時
            led(90);                        // RGB LEDを点灯(輝度90)
        }else{
            led(20);                        // RGB LEDを暗く(輝度20)
        }
        int i=0;
        while(!digitalRead(PIN_BTN)){
            delay(100);
            i++;
            if(i==10){
                led_num(((uint32_t)(WiFi.localIP()))>>24);  // IPアドレス
            }
            if(i==20){
                led(120,0,0);
            }
            if(i==30){
                led(0,120,0);
            }
            if(i==40){
                led(0,0,120);
            }
            if(i==50){
                led(100,80,70);
            }
        }
    }
    server.handleClient();                  // クライアントからWebサーバ呼び出し
}
