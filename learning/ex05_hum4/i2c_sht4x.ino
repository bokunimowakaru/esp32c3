/*********************************************************************
本ソースリストおよびソフトウェアは、ライセンスフリーです。(詳細は別記)
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

I2C接続の温湿度センサの値を読み取る
SENSIRION社 SHT4x

参考文献:
SENSIRION Datasheet, 
SHT4x 4th Gen. Relative Humidity and Temperature Sensor, 
Version 6.2, August 2023

                               Copyright (c) 2023 Wataru KUNINO
                               https://bokunimo.net/bokunimowakaru/
*********************************************************************/

#include <Wire.h> 
#define I2C_sht 0x44            // SHT4x の I2C アドレス 

float _i2c_sht4x_hum;

float getTemp(){
    int temp,hum;
    _i2c_sht4x_hum=-999.;
    Wire.beginTransmission(I2C_sht);
    Wire.write(0xFD);
    if( Wire.endTransmission() == 0){
        delay(18);                  // 15ms以上
        Wire.requestFrom(I2C_sht,6);
        if(Wire.available()==0) return -999.;
        temp = Wire.read();
        temp <<= 8;
        if(Wire.available()==0) return -999.;
        temp += Wire.read();
        if(Wire.available()==0) return -999.;
        Wire.read();
        if(Wire.available()==0) return -999.;
        hum = Wire.read();
        hum <<= 8;
        if(Wire.available()==0) return -999.;
        hum += Wire.read();
        if(Wire.available()==0) return -999.;
        Wire.read();
        Serial.printf("debug temp=0x%04X hum=0x%04X\n", temp,hum);
        _i2c_sht4x_hum = -6. + (float)hum / 65535. * 125.;
        if(_i2c_sht4x_hum < 0.)   _i2c_sht4x_hum = 0.;
        if(_i2c_sht4x_hum > 100.) _i2c_sht4x_hum = 100.;
        return (float)temp / 65535. * 175. - 45.;
    }else return -999.;
}

float getHum(){
    return _i2c_sht4x_hum;
}

void shtSetup(int sda,int scl){
    delay(2);                   // 1ms以上
    Wire.begin(sda,scl);        // I2Cインタフェースの使用を開始
    delay(18);                  // 15ms以上
}

void shtSetup(){
    delay(2);                   // 1ms以上
    Wire.begin();               // I2Cインタフェースの使用を開始
    delay(18);                  // 15ms以上
}