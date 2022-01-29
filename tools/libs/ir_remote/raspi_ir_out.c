/*******************************************************************************
Raspberry Pi用 赤外線リモコン送信プログラム  raspi_ir_out

指定したGPIOのポートから赤外線リモコン信号を送信します。

    使い方：

        $ raspi_ir_out ポート番号 方式 秒数

    使用例：

        $ raspi_ir_out 17 0 AA 5A 8F 12 16 D1      	AEHA方式のリモコン信号を送信
        $ raspi_ir_out 17 1 ～      				NEC方式のリモコン信号を送信
        $ raspi_ir_out 17 2 ～      				SIRC方式のリモコン信号を送信
        $ raspi_ir_out 17 -1						ポートを不使用に戻す

    戻り値
        0       正常終了
        -1      異常終了
                                        Copyright (c) 2015-2017 Wataru KUNINO
                                        https://bokunimo.net/raspi/
                                        
WiringPiをインストールする方法


git clone git://git.drogon.net/wiringPi
cd wiringPi
./build

gcc -Wall -O1 -lwiringPi raspi_ir_out.c  -o raspi_ir_out
                                        
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>         // usleep用

#define IR_MODE   	0       // 外線信号のモード（0はAEHA方式）
    						// enum IR_TYPE{AEHA=0,NEC=1,SIRC=2};
#define DATA_SIZE   32      // 送信最大データサイズ（バイト）
#define RasPi_1_REV 2       // Raspberry Pi 1 Type B の場合のリビジョン 通常=2
#define RasPi_PORTS 26      // Raspberry Pi GPIO ピン数 26 固定
#define GPIO_RETRY  3       // GPIO 切換え時のリトライ回数
#define S_NUM       8       // 文字列の最大長
	#define DEBUG           // デバッグモード

typedef unsigned char byte; 
FILE *fgpio;
char gpio[]="/sys/class/gpio/gpio00/value";
char dir[] ="/sys/class/gpio/gpio00/direction";
int IR_OUT;
#include "ir_send.c"

int ahex2i(char c){
    if(c>='0' && c<='9') return (int)(c-'0');
    if(c>='a' && c<='f') return (int)(c-'a'+10);
    if(c>='A' && c<='F') return (int)(c-'A'+10);
    return -1;
}

int main(int argc,char **argv){
    byte data[DATA_SIZE];	// 赤外線リモコン信号用
    int i;              	// ループ用
    int port;           	// GPIOポート
    int value;          	// 応答値
    int len=0;				// 送信データ長
    int mode=IR_MODE;  		// 赤外線信号のモード（0はAEHA方式）

    #if RasPi_1_REV == 1
        /* RasPi      pin 1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16    */
        int pin_ports[]={-1,-1, 0,-1, 1,-1, 4,14,-1,15,17,18,21,-1,22,23,
        /* BCM           17 18 19 20 21 22 23 24 25 26                      */
                         -1,24,10,-1, 9,25,11, 8,-1, 7};
        int wpi_ports[]={-1,-1, 8,-1, 9,-1, 7,15,-1,16, 0, 1, 2,-1, 3, 4,
        /* wPi           17 18 19 20 21 22 23 24 25 26                      */
                         -1, 5,12,-1,13, 6,14,10,-1,11};
    #else
        /* Pi B Rev1  pin 1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16    */
        int pin_ports[]={-1,-1, 2,-1, 3,-1, 4,14,-1,15,17,18,27,-1,22,23,
        /* BCM           17 18 19 20 21 22 23 24 25 26                      */
                         -1,24,10,-1, 9,25,11, 8,-1, 7};
        int wpi_ports[]={-1,-1, 8,-1, 9,-1, 7,15,-1,16, 0, 1, 2,-1, 3, 4,
        /* wPi           17 18 19 20 21 22 23 24 25 26                      */
                         -1, 5,12,-1,13, 6,14,10,-1,11};
    #endif
    
    if( argc==3 && atoi(argv[2]) == -1 ){
	    #ifdef DEBUG
	        printf("unexport\n");
	    #endif
	}else if( argc < 5 || argc > 3 + DATA_SIZE ){
        fprintf(stderr,"usage: %s port mode data1 data2 ...\n",argv[0]);
        printf("9\n");
        return -1;
    }
    /* 第1引数portの内容確認と設定 */
    port = atoi(argv[1]);
    for(i=0;i<RasPi_PORTS;i++){
        if( pin_ports[i] == port ) break;
    }
    if(RasPi_1_REV==2 && port==13){
		i=32;							// pin 33
		IR_OUT=23;						// wPi 23
	}else{
		IR_OUT=wpi_ports[i];			// wPi ポートを設定
	}
    if( i==RasPi_PORTS || port<0 || IR_OUT<0 ){
        fprintf(stderr,"Unsupported Port Error, %d\n",port);
        printf("9\n");
        return -1;
    }
    #ifdef DEBUG
        printf("Pin = %d, Port(BCM) = %d Port(wPi) = %d\n",i+1,port,IR_OUT);
    #endif
    /* 第2引数valueの内容確認と設定 */
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
            mode=value;
            break;
        default:
            fprintf(stderr,"Unsupported Value Error, %d\n",value);
            printf("9\n");
            return -1;
    }
    #ifdef DEBUG
        printf("mode = %d\n",mode);
    #endif
    
    /* 第3引数以降のdataの取り込み */
    for(i=3;i<argc;i++){
		value=ahex2i((char)argv[i][1]);
		if(value>=0) value += ahex2i((char)argv[i][0])*16;
		if(value>=0){
			data[len]=(byte)value;
			len++;
			if(len>=DATA_SIZE) len= DATA_SIZE-1;	// メモリ保護のための処理
		}
	}
    #ifdef DEBUG
        printf("data[%d] = ",len);
        for(i=0;i<len;i++) printf("%02X ",data[i]);
        printf("\n");
    #endif
    
    /* ポート番号の設定 */
    gpio[20]='\0';
    dir[20]='\0';
    sprintf(gpio,"%s%d/value",gpio,port);
    sprintf(dir,"%s%d/direction",dir,port);
    /* ポート開始処理 */
    
    /*
    fgpio = fopen(gpio, "w");
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
            fprintf(fgpio,"out\n");
            fclose(fgpio);
            #ifdef DEBUG
                printf("Set Direction to OUT (tried %d)\n",i);
            #endif
            fgpio = fopen(gpio, "w");
            if(fgpio==NULL){
                fprintf(stderr,"IO Error %s\n",gpio);
                printf("9\n");
                return -1;
            }
        }
    }
    */
    /* ポート出力処理 */
    /*
    fprintf(fgpio,"%d\n",IR_OUT_OFF);
    fclose(fgpio);
    */
    ir_init();
    /* 赤外線リモコン信号の送信処理 */
    ir_send(data, (byte)len, (byte)mode );
    return 0;
}
