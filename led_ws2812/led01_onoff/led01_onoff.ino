/***********************************************************************
led01_onoff.ino RGB LED WS2812
フルカラーLED を ON / OFF
                                        Copyright (c) 2021 Wataru KUNINO
***********************************************************************/
#define PIN_LED 8                               // IO 8 にLEDを接続する
#define T_Delay 360                             // LED処理による遅延(ns)
#define T0H_ns 320                              // b0信号Hレベル時間(ns)
#define T0L_ns 1200 -320                        // b0信号Lレベル時間(ns)
#define T1H_ns 640                              // b1信号Hレベル時間(ns)
#define T1L_ns 1200 -640                        // b1信号Lレベル時間(ns)

int T0H_num,T0L_num,T1H_num,T1L_num;            // 待ち時間カウンタ値

/* 引数nsに代入された待ち時間(ns)に対応する待ち時間処理回数を求める */
int _led_delay(int ns){                         // カウンタ設定処理部
    volatile uint32_t i;                        // 繰り返し処理用変数t
    uint32_t target, counts=0;                  // 目標時刻,試行繰返し数
    ns -= T_Delay;                              // 処理遅延分を減算
    noInterrupts();                             // 割り込みの禁止
    do{                                         // 繰り返し処理の開始
        i = ++counts;                           // 試行回数を増やしてiに
        target = micros() + ns / 10;            // 目標時刻を設定
        while(i>0) i--;                         // 待ち時間処理の実行
    }while(micros() < target);                  // 目標未達成時に繰返し
    interrupts();                               // 割り込みの許可
    return (counts + 50)/100;                   // 繰り返し回数を応答
}

/* 引数r,g,bに代入された色をLEDに送信する。値は0～255の範囲で設定 */
void led(int r,int g,int b){                    // LEDにカラーを設定
    digitalWrite(PIN_LED,LOW);                  // Lレベル
    delayMicroseconds(300);                     // 280us以上を維持
    volatile int TH, TL;                        // H/Lレベル時間保持用
    uint32_t rgb = (g & 0xff) << 16 | (r & 0xff) << 8 | (b & 0xff);
    noInterrupts();                             // 割り込みの禁止
    for(int b=23;b >= 0; b--){                  // 全24ビット分の処理
        if(rgb & (1<<b)){                       // 対象ビットが1のとき
            TH = T1H_num;                       // Hレベルの待ち時間設定
            TL = T1L_num;                       // Hレベルの待ち時間設定
        }else{                                  // 対象ビットが0のとき
            TH = T0H_num;                       // Lレベルの待ち時間設定
            TL = T0L_num;                       // Lレベルの待ち時間設定
        }
        digitalWrite(PIN_LED,HIGH);             // Hレベルを出力
        while(TH>0) TH--;                       // 待ち時間処理
        digitalWrite(PIN_LED,LOW);              // Lレベルを出力
        while(TL>0) TL--;                       // 待ち時間処理
    }
    interrupts();                               // 割り込みの許可
}

/* 初期化処理 */
void setup() {                                  // 一度だけ実行する関数
    pinMode(PIN_LED,OUTPUT);                    // ポートを出力に設定
    T0H_num=_led_delay(T0H_ns);                 // 待ち時間処理回数変換
    T0L_num=_led_delay(T0L_ns);
    T1H_num=_led_delay(T1H_ns);
    T1L_num=_led_delay(T1L_ns);
}

/* LEDの点滅処理 */
void loop() {                                   // 繰り返し実行する関数
    led(5,10,2);                                // LED ON
    delay(1000);                                // 1秒間の待ち時間処理
    led(0,0,0);                                 // LED OFF
    delay(1000);                                // 1秒間の待ち時間処理
}
