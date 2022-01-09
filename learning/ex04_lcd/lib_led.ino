/***********************************************************************
LED制御ドライバ RGB LED WS2812
解説＝ https://bokunimo.net/blog/esp/1522/
                                        Copyright (c) 2021 Wataru KUNINO
***********************************************************************/
#define T0H_ns 320                              // b0信号Hレベル時間(ns)
#define T0L_ns 1200 -320                        // b0信号Lレベル時間(ns)
#define T1H_ns 640                              // b1信号Hレベル時間(ns)
#define T1L_ns 1200 -640                        // b1信号Lレベル時間(ns)

int _PIN_LED = 8;
int T_Delay,T0H_num,T0L_num,T1H_num,T1L_num;    // 待ち時間カウンタ値

/* 引数nsに代入された待ち時間(ns)に対応する待ち時間処理回数を求める */
int _led_delay(int ns){                         // カウンタ設定処理部
    volatile uint32_t i;                        // 繰り返し処理用変数i
    uint32_t target, counts=0;                  // 目標時刻,試行繰返し数
    ns -= T_Delay;                              // 処理遅延分を減算
    // portMUX_TYPE mutex = portMUX_INITIALIZER_UNLOCKED;  // 排他制御用
    // portENTER_CRITICAL_ISR(&mutex);             // 割り込みの禁止
    do{                                         // 繰り返し処理の開始
        i = ++counts;                           // 試行回数を増やしてiに
        target = micros() + ns / 10;            // 目標時刻を設定
        while(i>0) i--;                         // 待ち時間処理の実行
    }while(micros() < target);                  // 目標未達成時に繰返し
    // portEXIT_CRITICAL_ISR(&mutex);              // 割り込み許可
    return (counts + 50)/100;                   // 繰り返し回数を応答
}

/* 信号操作に必要な処理時間を算出する。戻り値は必要時間(ns) */
int _initial_delay(){                           // 初期ディレイ測定部
    volatile uint32_t i=0;                      // 繰り返し処理用変数i
    uint32_t start, t, counts;                  // 開始時刻,試行繰返し数
    // portMUX_TYPE mutex = portMUX_INITIALIZER_UNLOCKED;  // 排他制御用
    // portENTER_CRITICAL_ISR(&mutex);             // 割り込みの禁止
    start = micros();                           // 開始時刻の保持
    counts = 0;                                 // カウンタのリセット
    do{                                         // 繰り返し処理の開始
        counts++;                               // カウント
    }while(counts < 1000);                      // 目標未達成時に繰返し
    t = micros() - start;                       // 経過時間をtに代入
    start = micros();                           // 開始時刻の保持
    counts = 0;                                 // カウンタのリセット
    do{                                         // 繰り返し処理の開始
        counts++;                               // カウント
        digitalWrite(_PIN_LED,LOW);             // (被測定対象)GPIO制御
        while(i>0);                             // (被測定対象)while
    }while(counts < 1000);                      // 目標未達成時に繰返し
    t = micros() - start - t;                   // 対象処理に要した時間
    // portEXIT_CRITICAL_ISR(&mutex);              // 割り込み許可
    return t;                                   // 繰り返し回数を応答
}

/* 引数r,g,bに代入された色をLEDに送信する。値は0～255の範囲で設定 */
void led(int r,int g,int b){                    // LEDにカラーを設定
    digitalWrite(_PIN_LED,LOW);                 // Lレベル
    delayMicroseconds(300);                     // 280us以上を維持
    volatile int TH, TL;                        // H/Lレベル時間保持用
    uint32_t rgb = (g & 0xff) << 16 | (r & 0xff) << 8 | (b & 0xff);

    // vTaskSuspendAll();                          // OSによるSwapOutの防止
    //  https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3
    //                     /api-reference/system/freertos.html#task-api
    yield();                                    // 割り込み動作
    portMUX_TYPE mutex = portMUX_INITIALIZER_UNLOCKED;  // 排他制御用
    portENTER_CRITICAL_ISR(&mutex);             // 割り込みの禁止
    for(int b=23;b >= 0; b--){                  // 全24ビット分の処理
        if(rgb & (1<<b)){                       // 対象ビットが1のとき
            TH = T1H_num;                       // Hレベルの待ち時間設定
            TL = T1L_num;                       // Hレベルの待ち時間設定
        }else{                                  // 対象ビットが0のとき
            TH = T0H_num;                       // Lレベルの待ち時間設定
            TL = T0L_num;                       // Lレベルの待ち時間設定
        }
        if(TH){                                 // THが0以外の時
            digitalWrite(_PIN_LED,HIGH);        // Hレベルを出力
            while(TH>0) TH--;                   // 待ち時間処理
            digitalWrite(_PIN_LED,LOW);         // Lレベルを出力
            while(TL>0) TL--;                   // 待ち時間処理
        }else{                                  // THが0の時
            digitalWrite(_PIN_LED,HIGH);        // Hレベルを出力
            digitalWrite(_PIN_LED,LOW);         // Lレベルを出力
            while(TL>0) TL--;                   // 待ち時間処理
        }
    }
    portEXIT_CRITICAL_ISR(&mutex);              // 割り込み許可
    // if(!xTaskResumeAll()) taskYIELD();       // OSの再開
}

void led(int brightness){                       // グレースケール制御
    if(brightness > 0xff) brightness = 0xff;    // 256以上時に255に設定
    led(brightness,brightness,brightness);      // RGB全て同値でLED制御
}

void led_off(){                                 // LED制御の停止
    led(0);                                     // LEDの消灯
    digitalWrite(_PIN_LED,LOW);                 // リセット(Lレベル)
    delayMicroseconds(300);                     // 280us以上を維持
}

void led_setup(int pin){
    _PIN_LED = pin;
    pinMode(_PIN_LED,OUTPUT);                   // ポートを出力に設定
    digitalWrite(_PIN_LED,LOW);
    delay(100);
    T_Delay = _initial_delay();                 // 信号処理遅延を算出
    T0H_num=_led_delay(T0H_ns);                 // 待ち時間処理回数変換
    T0L_num=_led_delay(T0L_ns);
    T1H_num=_led_delay(T1H_ns);
    T1L_num=_led_delay(T1L_ns);
    led_off();
}

void led_setup(){
    led_setup(_PIN_LED);
}
