/*******************************************************************************
Raspberry Pi用 赤外線リモコン受信プログラム  raspi_ir_in

指定したGPIOのポートから赤外線リモコン信号(AEHA)を取得します。

    使い方：

        $ ./raspi_ir_in ポート番号 方式 秒数

    使用例：

        $ ./raspi_ir_in 17          GIPOポート17から赤外線リモコン信号を取得
        $ ./raspi_ir_in 17 -1       GIPOポート17を解放
        $ ./raspi_ir_in 17 0        AEHA方式(Panasonic,Sharp)のリモコン信号を取得
        $ ./raspi_ir_in 17 1        NEC方式(Onkyo)のリモコン信号を取得
        $ ./raspi_ir_in 17 2        SIRC方式(SONY)のリモコン信号を取得
        $ ./raspi_ir_in 17 255      方式を自動選択してからリモコン信号を取得
        $ ./raspi_ir_in 17 0 10     10秒間、待ち続けて、信号が無ければタイムアウト

    応答値(stdio)
        取得したリモコン信号

    戻り値
        0       正常終了
        -1      異常終了
                                        Copyright (c) 2015-2019 Wataru KUNINO
                                        https://bokunimo.net/raspi/
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>         // usleep用

#define IR_MODE     255     // 外線信号のモード（0はAEHA方式、255は自動）
                            // enum IR_TYPE{AEHA=0,NEC=1,SIRC=2};
#define TIMEOUT     -1      // 受信のタイムアウト設定（秒）、-1で∞
#define DATA_SIZE   32      // 受信データサイズ（バイト）

#define RasPi_1_REV 2       // Raspberry Pi 1 Type B の場合のリビジョン 通常=2
#define RasPi_PORTS 26      // Raspberry Pi GPIO ピン数 26 固定
#define GPIO_RETRY  3       // GPIO 切換え時のリトライ回数
#define S_NUM       8       // 文字列の最大長
//  #define DEBUG           // デバッグモード

typedef unsigned char byte; 
FILE *fgpio;
char buf[S_NUM];            // ir_read.c内のdigitalReadで使用するために確保
char gpio[]="/sys/class/gpio/gpio00/value";
char dir[] ="/sys/class/gpio/gpio00/direction";
//// wipi[]="/usr/local/bin/gpio -g mode 00 up/down/tri";   // wipi[28-29]
char wipi[]="/usr/bin/gpio -g mode 00 up/down/tri";   // wipi[28-29]
//// 6文字減る　2019/10

#include "ir_read.c"

int main(int argc,char **argv){
    char s[S_NUM];
    byte data[DATA_SIZE];   // 赤外線リモコン信号用
    int i;                  // ループ用
    int port;               // GPIOポート
    int value;              // 応答値
    int mode=IR_MODE;       // 赤外線信号のモード（0はAEHA方式）
    int time=TIMEOUT;       // タイムアウト管理用

    #if RasPi_1_REV == 1
        /* RasPi      pin 1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16    */
        int pin_ports[]={-1,-1, 0,-1, 1,-1, 4,14,-1,15,17,18,21,-1,22,23,
        /*               17 18 19 20 21 22 23 24 25 26                      */
                         -1,24,10,-1, 9,25,11, 8,-1, 7};
    #else
        /* Pi B Rev1  pin 1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16    */
        int pin_ports[]={-1,-1, 2,-1, 3,-1, 4,14,-1,15,17,18,27,-1,22,23,
        /*               17 18 19 20 21 22 23 24 25 26                      */
                         -1,24,10,-1, 9,25,11, 8,-1, 7};
    #endif
    
    if( argc < 2 || argc > 4 ){
        fprintf(stderr,"usage: %s port [[value] wait(sec.) ]\n",argv[0]);
        printf("9\n");
        return -1;
    }
    /* 第1引数portの内容確認と設定 */
    port = atoi(argv[1]);
    for(i=0;i<RasPi_PORTS;i++){
        if( pin_ports[i] == port ){
            #ifdef DEBUG
                printf("Pin = %d, Port = %d\n",i+1,port);
            #endif
            break;
        }
    }
    if( i==RasPi_PORTS || port<0 ){
        fprintf(stderr,"Unsupported Port Error, %d\n",port);
        printf("9\n");
        return -1;
    }
        
    /* 第2引数valueの内容確認と設定 */
    if( argc >= 3 ){
        value = atoi(argv[2]);
        switch( value ){
            case -1:
                fgpio = fopen("/sys/class/gpio/unexport","w");
                if(fgpio){
                    fprintf(fgpio,"%d\n",port);
                    fclose(fgpio);
                    #ifdef DEBUG
                        printf("Disabled Port\n");
                    #else
                        printf("-1\n");
                    #endif
                    return 0;
                }else{
                    fprintf(stderr,"IO Error\n");
                    printf("9\n");
                    return -1;
                }
                break;
            case 0:
            case 1:
            case 2:
            case 255:
                mode=value;
                break;
            default:
                fprintf(stderr,"Unsupported Value Error, %d\n",value);
                printf("9\n");
                return -1;
        }
    }
    #ifdef DEBUG
        printf("mode = %d\n",mode);
    #endif
    
    /* 第3引数timeの内容確認と設定 */
    if( argc >= 4 ){
        time = atoi(argv[3]);
    }
    #ifdef DEBUG
        printf("time = %d\n",time);
    #endif
    
    /* ポート番号の設定 */
    gpio[20]='\0';
    dir[20]='\0';
    wipi[28-6]='\0';
    sprintf(gpio,"%s%d/value",gpio,port);
    sprintf(dir,"%s%d/direction",dir,port);
    sprintf(wipi,"%s %d tri",wipi,port);
    /* ポート開始処理 */
    fgpio = fopen(gpio, "r");
    if( fgpio==NULL ){
        fgpio = fopen("/sys/class/gpio/export","w");
        if(fgpio==NULL ){
            fprintf(stderr,"IO Error\n");
            printf("9\n");
            return -1;
        }else{
            fprintf(fgpio,"%d\n",port);
            fclose(fgpio);
            #ifdef DEBUG
                printf("Enabled Port\n");
            #endif
            for(i=0;i<GPIO_RETRY;i++){
                fgpio = fopen(dir, "w");
                if( fgpio ) break;
                usleep(50000);
            }
            if(i==GPIO_RETRY){
                fprintf(stderr,"IO Error %s\n",dir);
                printf("9\n");
                return -1;
            }
            fprintf(fgpio,"in\n");
            fclose(fgpio);
            #ifdef DEBUG
                printf("Set Direction to IN (tried %d)\n",i);
            #endif
            fgpio = fopen(gpio, "r");
            if(fgpio==NULL){
                fprintf(stderr,"IO Error %s\n",gpio);
                printf("9\n");
                return -1;
            }
        }
    }
    
    /* トリステート設定(2017/2/19追加) */
    system(wipi);
    
    /* ポート入力処理 */
    fgets(s, S_NUM, fgpio);
    value = atoi(s);
    fclose(fgpio);
    /* 赤外線リモコン信号の待ち受け処理 */
    do{
        while( value ){
            value = digitalRead();
            if( time >= 0 && micros() > time * 1000000){
                #ifdef DEBUG
                    printf("Timeout (no data)\n");
                #endif
                printf("00 \n");
                return 0;
            }
        }
        value = ir_read(data,DATA_SIZE,mode);
        if( value > 0 ){
            if(value%8) value += 8;
            value /= 8;
        }
        #ifdef DEBUG
        else printf("Detected noise (%d)\n",value);
        #endif
    }while( value <= 0 );
    #ifdef DEBUG
    printf("len     = %d\n",value);
    #endif
    for(i=0;i<value;i++){
        printf("%02X ",data[i]);
    }
    printf("\n");
    return 0;
}
