/*******************************************************************************
Example 0: ESP32C3 IOボード + RGB LED でGPIO制御 【RGB LED 対応版】

                                          Copyright (c) 2021-2023 Wataru KUNINO
*******************************************************************************/

#define PIN_LED 0                           // IO0にLEDを接続
// #define PIN_LED 10                       // IO10にLEDを接続(ピッチ変換基板用)
#define PIN_SW 1                            // IO1にタクトスイッチを接続
#define PIN_LED_RGB 2                       // IO2にWS2812を接続(m5stamp用)
// #define PIN_LED_RGB 8                    // IO8にWS2812(ピッチ変換基板用)

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED, OUTPUT);               // (通常の)LED用のIOポートを出力に
    pinMode(PIN_SW,INPUT_PULLUP);           // タクトスイッチ入力の設定
    led_setup(PIN_LED_RGB);                 // WS2812の初期設定(ポートを設定)
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32C3 IO TEST");      // 「IO TEST」をシリアル出力表示
    morse(PIN_LED,50,"HELLO");              // モールス信号(ピン,速度50,HELLO)
    while(millis() < 10000){                // 起動後10秒間
        digitalWrite(PIN_LED,millis()/100%2); // LEDの点滅
        led((millis()/50) % 10);            // WS2812の点滅
        delay(50);                          // 待ち時間処理
    }
    Serial.println("Ready");                // 「Ready」をシリアル出力
    digitalWrite(PIN_LED, LOW);             // LEDの消灯
    led(0);                                 // WS2812を消灯
}

void loop(){                                // 繰り返し実行する関数
    int in = digitalRead(PIN_SW);           // 変数inにタクトスイッチ状態を代入
    if(in ==0){                             // IO1がLowレベル(押下状態)のとき
        Serial.println("LED ON");           // 「LED ON」をシリアル出力
        digitalWrite(PIN_LED, HIGH);        // IO0をHighレベル(LED点灯)に設定
        led(0,20,20);                       // WS2812を点灯(水色20)
        int i = 0;                          // ループ用の数値変数iを定義
        while(i<100){                       // スイッチ・ボタン解除待ち
            i = digitalRead(PIN_SW)? i+1:0; // ボタン開放時にiに1を加算
            delay(1);                       // 待ち時間処理
        }
        digitalWrite(PIN_LED, LOW);         // IO0をLowレベル(LED消灯)に設定
        led(0);                             // WS2812を消灯
    }
}
