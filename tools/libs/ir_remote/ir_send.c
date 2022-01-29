/*********************************************************************

赤外線リモコン送信部 for Raspberry Pi

本ソースリストおよびソフトウェアは、ライセンスフリーです。
個人での利用は自由に行えます。著作権表示の改変は禁止します。

                               Copyright (c) 2012-2020 Wataru KUNINO
                               https://bokunimo.net/raspi/
*********************************************************************/
/*
赤外線リモコン信号を送信します。
*/

// #define IR_OUT		12				// 赤外線LEDの接続ポート
extern int IR_OUT;

#define IR_OUT_OFF	0				// 赤外線LED非発光時の出力値
#define IR_OUT_ON	1				// 赤外線LED発光時の出力値
//	#define DATA_SIZE   32      	// 送信最大データサイズ（バイト）

// Raspberry Pi 4向けのパラメータ（処理遅延の考慮無し）
#define FLASH_AEHA_TIMES	18	// シンボルの搬送波点滅回数（ＡＥＨＡ 470 us）
#define FLASH_NEC_TIMES		21	// シンボルの搬送波点滅回数（ＮＥＣ 560 us）
#define FLASH_SIRC_TIMES	23	// シンボルの搬送波点滅回数（ＳＩＲＣ 600 us）
#define FLASH_ON			13	// LED ON 期間 us (規格上 ON+OFFで 26 us)
#define FLASH_OFF			13	// LED ON 期間 us (規格上 ON+OFFで 26 us)

/* 処理速度が遅い場合のパラメータ
#define FLASH_AEHA_TIMES	16	// シンボルの搬送波点滅回数（ＡＥＨＡ）
#define FLASH_NEC_TIMES		22	// シンボルの搬送波点滅回数（ＮＥＣ）
#define FLASH_SIRC_TIMES	24	// シンボルの搬送波点滅回数（ＳＩＲＣ）
#define FLASH_ON			11	// LED ON 期間 us (規格上 ON+OFFで 23 us)
#define FLASH_OFF			11	// LED ON 期間 us (規格上 ON+OFFで 23 us)
*/

// enum IR_TYPE{ AEHA=0, NEC=1, SIRC=2 };		// 家製協AEHA、NEC、SONY SIRC切り換え
#define AEHA		0
#define NEC			1
#define SIRC		2

#include <unistd.h>							// sleep
#include <wiringPi.h>

// 置換	delayMicroseconds 
					//   012345678901234567890
extern char gpio[]; 	// ="/sys/class/gpio/gpio00/value";

/* 速度が間に合わなかった
byte digitalWrite(byte port,byte value){
    gpio[20]='\0';
	sprintf(gpio,"%s%d/value",gpio,port);
    fgpio = fopen(gpio, "w");
    if(fgpio){
	    fprintf(fgpio,"%d\n",value);
	    fclose(fgpio);
	    return (byte)value;
	}
	#ifdef DEBUG
		printf("IO ERROR(%s port=%d,val=%d)\n",gpio,port,value);
	#endif
	return 255;
}
*/

void ir_init(void){
	wiringPiSetup();
	pinMode(IR_OUT, OUTPUT);
	digitalWrite(IR_OUT, IR_OUT_OFF);
}

/* 赤外線ＬＥＤ点滅用 */
void ir_flash(byte times){
	while(times){
		times--;
		delayMicroseconds(FLASH_ON);				// 13 uS
		digitalWrite(IR_OUT, IR_OUT_ON);
		delayMicroseconds(FLASH_OFF);				// 13 uS
		digitalWrite(IR_OUT, IR_OUT_OFF);
	}
}
void ir_wait(byte times){
	while(times){
		times--;
		delayMicroseconds(FLASH_ON);				// 13 uS
		digitalWrite(IR_OUT, IR_OUT_OFF);
		delayMicroseconds(FLASH_OFF);				// 13 uS
		digitalWrite(IR_OUT, IR_OUT_OFF);
	}
}

/* 赤外線ＬＥＤ信号送出 */
void ir_send(byte *data, const byte data_len, const byte ir_type ){
	byte i,j,t;
	byte b;
	
	if(data_len<2) return;
	switch( ir_type ){
		case NEC:
			ir_flash( 8 * FLASH_NEC_TIMES );	// send 'SYNC_H'
			ir_flash( 8 * FLASH_NEC_TIMES );
			ir_wait(  8 * FLASH_NEC_TIMES );	// send 'SYNC_L'
			for( i = 0 ; i < data_len ; i++){
				for( b = 0 ; b < 8 ; b++ ){
					ir_flash( FLASH_NEC_TIMES );
					if( data[i] & (0x01 << b) ){
						ir_wait( 3 * FLASH_NEC_TIMES );
					}else{
						ir_wait( FLASH_NEC_TIMES );
					}
				}
			}
			ir_flash( FLASH_NEC_TIMES );		// send 'stop'
			break;
		case SIRC:
			for(j=0;j<3;j++){
				t=5;						// 送信済シンボル基本単位(SYNCで送信)
				ir_flash( 4 * FLASH_SIRC_TIMES );	// send 'SYNC_H'
				ir_wait(      FLASH_SIRC_TIMES );	// send 'SYNC_L'
				for( b = 0 ; b < 7 ; b++ ){
					if( data[0] & (0x01 << b) ){
						ir_flash( 2 * FLASH_SIRC_TIMES );
						t +=3 ;
					}else{
						ir_flash( FLASH_SIRC_TIMES );
						t +=2 ;
					}
					ir_wait( FLASH_SIRC_TIMES );
				}
				for( i = 1 ; i < data_len ; i++){
					for( b = 0 ; b < 8 ; b++ ){
						if( data[i] & (0x01 << b) ){
							ir_flash( 2 * FLASH_SIRC_TIMES );
							t +=3 ;
						}else{
							ir_flash( FLASH_SIRC_TIMES );
							t +=2 ;
						}
						ir_wait( FLASH_SIRC_TIMES );
					}
				}
				while( t <= 75 ){
					t++;
					ir_wait( FLASH_SIRC_TIMES );
				}
			}
			break;
		case AEHA:
		default:
			ir_flash( 8 * FLASH_AEHA_TIMES );	// send 'SYNC_H'
			ir_wait(  4 * FLASH_AEHA_TIMES);	// send 'SYNC_L'
			for( i = 0 ; i < data_len ; i++){
				for( b = 0 ; b < 8 ; b++ ){
					ir_flash( FLASH_AEHA_TIMES );
					if( data[i] & (0x01 << b) ){
						ir_wait( 3 * FLASH_AEHA_TIMES );
					}else{
						ir_wait( FLASH_AEHA_TIMES );
					}
				}
			}
			ir_flash( FLASH_AEHA_TIMES );		// send 'stop'
			break;
	}
}
