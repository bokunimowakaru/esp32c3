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

#define PIN_LED_RGB 1                       // G1にWS2812を接続(m5stamp Grove用)
// #define PIN_LED_RGB 2                    // G2にWS2812を接続(m5stamp内蔵LED)
// #define PIN_LED_RGB 8                    // G8にWS2812を接続(DevKitM内蔵LED)
#define PIN_BTN 9                           // G9にボタン(m5stampC3U用)
// #define PIN_BTN 3                        // G3にボタン(m5stampC3用)

int led_stat = 0;                           // LED状態用の変数led_statを定義

void setup(){                               // 起動時に一度だけ実行する関数
    Serial.begin(115200);                   // 動作確認用シリアル出力開始
    Serial.println("RGB LED WS2812");
    pinMode(PIN_BTN,INPUT_PULLUP);          // ボタン入力の設定
    led_setup(PIN_LED_RGB);                 // RGB LED の初期設定(ポートを設定)
    led_notify(1);                          // 起動をLEDで通知
    Serial.println("led=(20,40,10)");
    led(20,40,10);                          // RGB LEDを点灯 R=20, G=40, B=10
}

void loop(){                                // 繰り返し実行する関数
    if(!digitalRead(PIN_BTN)){
        led_notify(1);
        led_stat = !led_stat;
        if(led_stat){                       // led_statの絶対値が1以上の時
            Serial.println("led=90");
            led(90);                        // RGB LEDを点灯(輝度90)
        }else{
            Serial.println("led=20");
            led(20);                        // RGB LEDを暗く(輝度20)
        }
        int i=0;
        while(!digitalRead(PIN_BTN)){       // ボタン押下継続で1秒ごとに色変更
            delay(100);
            i++;
            if(i==10){
                Serial.println("led_num=123");
                led_num(123);
            }
            if(i==20){
                Serial.println("led=(120,0,0)");
                led(120,0,0);
            }
            if(i==30){
                Serial.println("led=(0,120,0)");
                led(0,120,0);
            }
            if(i==40){
                Serial.println("led=(0,0,120)");
                led(0,0,120);
            }
            if(i==50){
                Serial.println("led=(100,80,70)");
                led(100,80,70);
                i = 0;
            }
        }
    }
}
