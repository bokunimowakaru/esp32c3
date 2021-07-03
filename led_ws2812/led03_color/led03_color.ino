/***********************************************************************
led03_color RGB LED WS2812
フルカラーLED の 色を制御
                                        Copyright (c) 2021 Wataru KUNINO
***********************************************************************/


void tone2rgb(byte rgb[], int tone, int brightness){
    float r, g, b, v, f, q, t;
    v = (float)brightness / 255.;
    f = ((float)(tone % 60)) / 60.;
    t = f * v;
    q = v - t;
    switch(tone / 60){
        case 0: r=v; g=t; b=0; break;
        case 1: r=q; g=v; b=0; break;
        case 2: r=0; g=v; b=t; break;
        case 3: r=0; g=q; b=v; break;
        case 4: r=t; g=0; b=v; break;
        case 5: r=v; g=0; b=q; break;
        default: r=v; g=t; b=0; break;
    }
    rgb[0] = (byte)(r * 255. + .5);
    rgb[1] = (byte)(g * 255. + .5);
    rgb[2] = (byte)(b * 255. + .5);
}

/* 初期化処理 */
void setup() {                                  // 一度だけ実行する関数
    led_setup();                                // LEDドライバの初期化
}

/* LEDの制御 */
int tone = 0;                                   // 現在の色調(0〜359)
int tone_speed = +1;                            // 色調の変更速度
int brightness = 0;                             // 現在の輝度値
int dimmer_speed = +1;                          // 輝度の変更速度
int dimmer_max = 30;                            // 点灯時の輝度(255以下)
void loop() {                                   // 繰り返し実行する関数
    byte rgb[3];
    tone2rgb(rgb, tone, brightness);
    led(rgb[0],rgb[1],rgb[2]);                  // LED 制御
    brightness += dimmer_speed;                 // 輝度の増減
    tone += tone_speed;
    if(brightness < 0){                         // 輝度値が負になったとき
        brightness = 0;                         // 輝度値を0に設定
        dimmer_speed = abs(dimmer_speed);       // 正の速度を設定
    }else if(brightness > dimmer_max){          // 点灯時の輝度を超えたとき
        brightness = dimmer_max;                // 転倒時の輝度を設定
        dimmer_speed = -abs(dimmer_speed);      // 負の速度
    }
    if(tone >= 360) tone = 0;
    delay(14);                                  // 14msの待ち時間処理
}
