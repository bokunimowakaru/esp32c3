/*******************************************************************************
Example 0: ESP32C3 IOボードでGPIO制御

                                          Copyright (c) 2021-2022 Wataru KUNINO
*******************************************************************************/

#define PIN_LED 0                           // IO0 に (通常の)LED を接続
#define PIN_SW 1                            // IO1 にタクトスイッチを接続

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED, OUTPUT);               // (通常の)LED用のIOポートを出力に
    pinMode(PIN_SW,INPUT_PULLUP);           // タクトスイッチ入力の設定
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32C3 IO TEST");      // 「IO TEST」をシリアル出力表示
    while(millis() < 10000){                // 起動後10秒間
        digitalWrite(PIN_LED,millis()/100%2); // LEDの点滅
        delay(100);                         // 待ち時間処理
    }
    Serial.println("Read");                 // 「Ready」をシリアル出力
    digitalWrite(PIN_LED, LOW);             // LEDの消灯
}

void loop(){                                // 繰り返し実行する関数
    int in = digitalRead(PIN_SW);           // 変数inにタクトスイッチ状態を代入
    if(in ==0){                             // IO1がLowレベル(押下状態)のとき
        Serial.println("LED ON");           // 「LED ON」をシリアル出力
        digitalWrite(PIN_LED, HIGH);        // IO0をHighレベル(LED点灯)に設定
        int i = 0;                          // ループ用の数値変数iを定義
        while(i<100){                       // スイッチ・ボタン解除待ち
            i = digitalRead(PIN_SW)? i+1:0; // ボタン開放時にiに1を加算
            delay(1);                       // 待ち時間処理
        }
        digitalWrite(PIN_LED, LOW);         // IO0をLowレベル(LED消灯)に設定
    }
}
