/***********************************************************************
led_ws2812 RGB LED WS2812
                                        Copyright (c) 2021 Wataru KUNINO
***********************************************************************/
/*
参考文献
WS2812B Ver. No.: V5, Intelligent control LED, integrated light source
http://www.world-semi.com/
https://akizukidenshi.com/download/ds/worldsemi/WS2812B_20200225.pdf
*/
#define PIN_LED 8              // IO 8 にLEDを接続する
#define SK68XXMINI

#ifdef WS2812
    #define T0H_ns (220+380)/2
    #define T0L_ns (580+1000)/2
    #define T1H_ns (580+1000)/2
    #define T1L_ns (580+1000)/2
#endif

#ifdef SK68XXMINI
    #define T0H_ns 320 -200
    #define T0L_ns 1200 -320 -200
    #define T1H_ns 640 -200
    #define T1L_ns 1200 -640 -200
#endif

int T0H_num = 3;
int T0L_num = 9;
int T1H_num = 9;
int T1L_num = 9;

byte ledp[][3]={{10,10,10},{20,5,5},{5,20,5},{5,5,20}};

int _led_delay(int ns){
    volatile uint32_t i;
    uint32_t target, counts=0;
    delay(1000);
    noInterrupts();
    delay(1);
    do{
        i = ++counts;
        target = micros() + (uint32_t)ns / 10;
        while(i>0) i--;
    }while(micros() < target);
    interrupts();
    return (counts + 50)/100;
}

void _led_reset(){
    digitalWrite(PIN_LED,LOW);
    delayMicroseconds(300);     // 280us以上
}

void led(int r,int g,int b){
    _led_reset();
    volatile int TH, TL;
    uint32_t rgb = (g & 0xff) << 16 | (r & 0xff) << 8 | (b & 0xff);
    noInterrupts();
    for(int b=23;b >= 0; b--){
        if(rgb & (1<<b)){
            TH = T1H_num;
            TL = T1L_num;
        }else{
            TH = T0H_num;
            TL = T0L_num;
        }
        digitalWrite(PIN_LED,HIGH);
        while(TH>0) TH--;
        digitalWrite(PIN_LED,LOW);
        while(TL>0) TL--;
    }
    interrupts();
}

void setup() {                  // 起動時に一度だけ実行される関数
    Serial.begin(115200);
    Serial.println("RGB LED WS2812");
    pinMode(PIN_LED,OUTPUT);    // LEDを接続したポートを出力に設定する
    digitalWrite(PIN_LED,HIGH);
    T0H_num=_led_delay(T0H_ns);
    T0L_num=_led_delay(T0L_ns);
    T1H_num=_led_delay(T1H_ns);
    T1L_num=_led_delay(T1L_ns);
    Serial.printf("T0H %dns -> %d, ",T0H_ns,T0H_num);
    Serial.printf("T0L %dns -> %d\n",T0L_ns,T0L_num);
    Serial.printf("T1H %dns -> %d, ",T1H_ns,T1H_num);
    Serial.printf("T1L %dns -> %d\n",T1L_ns,T1L_num);
    led(0,0,0);
}

void loop() {                   // setup実行後に繰り返し実行される関数
    for(int i=0;i<sizeof(ledp)/sizeof(ledp[0]);i++){
        Serial.printf("p%d={%d,%d,%d}\n",i,ledp[i][0],ledp[i][1],ledp[i][2]);
        led(ledp[i][0],ledp[i][1],ledp[i][2]);
        delay(1000);            // 時間待ち(1秒)
    }
}
