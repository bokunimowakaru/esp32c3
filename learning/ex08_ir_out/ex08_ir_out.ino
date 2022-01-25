/*******************************************************************************
Example 51 (=32+19): ESP32 Wi-Fi コンシェルジェ リモコン担当(赤外線リモコン制御)
赤外線リモコンに対応した家電機器を制御します。 

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include <WebServer.h>                      // HTTPサーバ用ライブラリ

#define TIMEOUT 20000                       // タイムアウト 20秒
#define DATA_LEN_MAX 16                     // リモコンコードのデータ長(byte)
#define PIN_IR_IN 0                         // IO0 に IR センサを接続
#define PIN_IR_OUT 1                        // IO1 に IR LEDを接続
#define PIN_LED_RGB 2                       // IO2 に WS2812を接続(m5stamp)
// #define PIN_LED_RGB 8                    // IO8 に WS2812を接続(DevKitM)
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define PORT 1024                           // 送信のポート番号
#define DEVICE "ir_rc_1,"                   // デバイス名(5文字+"_"+番号+",")
#define AEHA        0                       // 赤外線送信方式(Panasonic、Sharp)
#define NEC         1                       // 赤外線送信方式 NEC方式
#define SIRC        2                       // 赤外線送信方式 SONY SIRC方式

IPAddress IP_BROAD;                         // ブロードキャストIPアドレス

WiFiUDP udp;                                // UDP通信用のインスタンスを定義
WebServer server(80);                       // Webサーバ(ポート80=HTTP)定義
byte D[DATA_LEN_MAX];                       // 保存用・リモコン信号データ
int D_LEN;                                  // 保存用・リモコン信号長（bit）
int IR_TYPE=AEHA;                           // リモコン方式

void handleRoot(){
    char s[97];                             // 文字列変数を定義 97バイト96文字
    Serial.println("Connected");            // 接続されたことをシリアル出力表示
    if(server.hasArg("TYPE")){              // 引数TYPEが含まれていた時
        IR_TYPE = server.arg("TYPE").toInt(); // 引数TYPEの値をIR_TYPEへ
    }
    if(server.hasArg("IR")){                // 引数IRが含まれていた時
        String rx = server.arg("IR");       // 引数IRの値を取得し変数rxへ代入
        Serial.println(rx);
        rx.toCharArray(s,97);
        trUri2txt(s);
        Serial.println(s);
        D_LEN=ir_txt2data(D,DATA_LEN_MAX,s); // 受信データsをリモコン信号に変換
        ir_send(D,D_LEN,IR_TYPE);
    }
    ir_data2txt(s, 97, D, D_LEN);           // 信号データDを表示文字sに変換
    String tx = getHtml(s,D_LEN,IR_TYPE);   // HTMLコンテンツを取得
    server.send(200, "text/html", tx);      // HTMLコンテンツを送信
}

void setup(){                               // 起動時に一度だけ実行する関数
    led_setup(PIN_LED_RGB);                 // WS2812の初期設定(ポート設定)
    ir_read_init(PIN_IR_IN);                // IRセンサの入力ポートの設定
    ir_send_init(PIN_IR_OUT);               // IR LEDの出力ポートの設定
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32C3 eg.8 ir_rc");   // タイトルをシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        led((millis()/50) % 10);            // (WS2812)LEDの点滅
        delay(50);                          // 待ち時間処理
    }
    morseIp0(-1,100,WiFi.localIP());        // IPアドレス終値をモールス信号出力
    server.on("/", handleRoot);             // HTTP接続時のコールバック先を設定
    server.begin();                         // Web サーバを起動する
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル表示
    IP_BROAD = WiFi.localIP();              // IPアドレスを取得
    IP_BROAD[3] = 255;                      // ブロードキャストアドレスに
    udp.begin(PORT);                        // UDP通信御開始
    led(0,20,0);                            // (WS2812)LEDを緑色で点灯
}

void loop(){
    byte d[DATA_LEN_MAX];                   // リモコン信号データ
    int d_len;                              // リモコン信号長（bit）
    char s[97];                             // 文字列変数を定義 97バイト96文字

    server.handleClient();                  // クライアントからWebサーバ呼出

    /* 赤外線受信・UDP送信処理 */
    d_len=ir_read(d,DATA_LEN_MAX,255);      // 赤外線信号を読み取る
    if(d_len>=16){                          // 16ビット以上の時に以下を実行
        led(40,0,0);                        // (WS2812)LEDを赤色で点灯
        udp.beginPacket(IP_BROAD, PORT);    // UDP送信先を設定
        udp.print(DEVICE);                  // デバイス名を送信
        udp.print(d_len);                   // 信号長を送信
        udp.print(",");                     // カンマ「,」を送信
        ir_data2txt(s,96,d,d_len);          // 受信データをテキスト文字に変換
        udp.println(s);                     // それを文字をUDP送信
        Serial.println(s);
        udp.endPacket();                    // UDP送信の終了(実際に送信する)
        memcpy(D,d,DATA_LEN_MAX);           // データ変数dを変数Dにコピーする
        D_LEN=d_len;                        // データ長d_lenをD_LENにコピーする
        delay(500);
        led(0,20,0);                        // (WS2812)LEDを緑色で点灯
    }
    /*
    d_len=udp.parsePacket();                // UDP受信長を変数d_lenに代入
    if(d_len==0)return;                     // TCPとUDPが未受信時にloop()先頭へ
    memset(s, 0, 97);                       // 文字列変数sの初期化(97バイト)
    udp.read(s, 96);                        // UDP受信データを文字列変数sへ代入
    if(
        d_len>6 && (                        // データ長が6バイトより大きくて、
            strncmp(s,"ir_rc_",6)==0 ||     // 受信データが「ir_rc_」
            strncmp(s,"ir_in_",6)==0        // または「ir_in_」で始まる時
        )
    ){
        D_LEN=ir_txt2data(D,DATA_LEN_MAX,&s[8]);
    }                                       // 受信TXTをデータ列に変換
    */
}
