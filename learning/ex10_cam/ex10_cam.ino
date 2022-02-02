/*******************************************************************************
Example 10: ESP32C3 Wi-Fi コンシェルジェ カメラ担当
 for SeeedStudio Grove Serial Camera Kit 
Webサーバ機能を使って、カメラのシャッターを制御し、撮影した写真を表示します。

    カメラ接続用
    IO7 TX カメラ側はRXD端子(白色)
    IO8 RX カメラ側はTXD端子(黄色)

    IO10 にPch-FETを接続

                                          Copyright (c) 2016-2022 Wataru KUNINO
*******************************************************************************/
// HardwareSerial.h
// void begin(unsigned long baud, uint32_t config=SERIAL_8N1, int8_t rxPin=-1, int8_t txPin=-1, bool invert=false, unsigned long timeout_ms = 20000UL, uint8_t rxfifo_full_thrhd = 112);
 
#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ

#define PIN_CAM 10                          // IO10 にPch-FETを接続
#define TIMEOUT 20000                       // タイムアウト 20秒
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define PORT 1024                           // UDP送信先ポート番号
#define DEVICE_CAM  "cam_a_1,"              // デバイス名(カメラ)

HardwareSerial hardwareSerial1(1);          // カメラ接続用シリアルポートESP32C3

WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義
IPAddress   IP_LOCAL;                       // 本機のIPアドレス
IPAddress   IP_BROAD;                       // ブロードキャストIPアドレス
int size=0;                                 // 画像データの大きさ(バイト)
int update=60;                              // ブラウザのページ更新間隔(秒)

void sendUdp(String dev, String S){
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    udp.beginPacket(IP_BROAD, PORT);        // UDP送信先を設定
    udp.println(dev + S);
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    Serial.println("udp://" + IP_BROAD.toString() + ":" + PORT + " " + dev + S);
    delay(200);                             // 送信待ち時間
}

void sendUdp_Fd(uint16_t fd_num){
    sendUdp(DEVICE_CAM, String(fd_num) + ", http://" + IP_LOCAL.toString() + "/cam.jpg");
}

void setup(){ 
    lcdSetup(8,2,1,0);                      // LCD初期(X=8,Y=2,SDA=1,SCL=0)
    pinMode(PIN_CAM,OPEN_DRAIN);            // FETを接続したポートをオープンに
    Serial.begin(115200);                   // 動作確認用用のシリアル出力開始
    Serial.println("Example 10 cam");       // 「Example 10」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    hardwareSerial1.begin(115200, SERIAL_8N1, 8, 7); 
    pinMode(PIN_CAM,OUTPUT);                // FETを接続したポートを出力に
    digitalWrite(PIN_CAM,LOW);              // FETをLOW(ON)にする
    delay(100);                             // 電源の供給待ち
    lcdPrint("Initializing");
    CamInitialize();                        // カメラの初期化コマンド
    lcdPrint("Setting QVGA");
    CamSizeCmd(1);                          // 撮影サイズをQVGAに設定
    lcdPrint("Done    settings");
    delay(4000);                            // 完了待ち(開始直後の撮影防止対策)
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
    }
    IP_LOCAL = WiFi.localIP();
    IP_BROAD = IP_LOCAL;
    IP_BROAD[3] = 255;
    server.begin();                         // サーバを起動する
    lcdPrintIp(IP_LOCAL);                   // 本機のIPアドレスを液晶に表示
    Serial.println(IP_LOCAL);               // 本機のIPアドレスをシリアル表示
    sendUdp_Fd(0);                          // 起動したことを知らせるUDP送信
}

void loop(){
    WiFiClient client;                      // Wi-Fiクライアントの定義
    char c;                                 // 文字変数を定義
    char s[65];                             // 文字列変数を定義 65バイト64文字
    int len=0;                              // 文字列等の長さカウント用の変数
    int t=0;                                // 待ち受け時間のカウント用の変数
    int i,j;
    
    client = server.available();            // 接続されたクライアントを生成
    if(!client)return;                      // loop()の先頭に戻る
    Serial.println("Connected");            // シリアル出力表示
    while(client.connected()){              // 当該クライアントの接続状態を確認
        if(client.available()){             // クライアントからのデータを確認
            t=0;                            // 待ち時間変数をリセット
            c=client.read();                // データを文字変数cに代入
            if(c=='\n'){                    // 改行を検出した時
                if(len>5 && strncmp(s,"GET /",5)==0) break;
                len=0;                      // 文字列長を0に
            }else if(c!='\r' && c!='\0'){
                s[len]=c;                   // 文字列変数に文字cを追加
                len++;                      // 変数lenに1を加算
                s[len]='\0';                // 文字列を終端
                if(len>=64) len=63;         // 文字列変数の上限
            }
        }
        t++;                                // 変数tの値を1だけ増加させる
        if(t>TIMEOUT) break; else delay(1); // TIMEOUTに到達したらwhileを抜ける
    }
    delay(1);                               // クライアント側の応答待ち時間
    if(!client.connected()||len<6) return;  // 切断された場合はloop()の先頭へ
    Serial.println(s);                      // 受信した命令をシリアル出力表示
    lcdPrint(&s[5]);                        // 受信した命令を液晶に表示
    if(strncmp(s,"GET / ",6)==0){           // コンテンツ取得命令時
        html(client,size,update,WiFi.localIP()); // コンテンツ表示
        client.flush();                     // ESP32用 ERR_CONNECTION_RESET対策
        client.stop();                      // クライアントの切断
        Serial.print(size);                 // ファイルサイズをシリアル出力表示
        Serial.println(" Bytes");           // シリアル出力表示
        return;                             // 処理の終了・loop()の先頭へ
    }
    if(strncmp(s,"GET /cam.jpg",12)==0){    // 画像取得指示の場合
        CamCapture();                       // カメラで写真を撮影する
        size=CamGetData(client);
        client.println("HTTP/1.0 200 OK");                  // HTTP OKを応答
        client.println("Content-Type: image/jpeg");         // JPEGコンテンツ
        client.println("Content-Length: " + String(size));  // ファイルサイズ
        client.println("Connection: close");                // 応答後に閉じる
        client.println();                                   // ヘッダの終了
        client.println();                   // コンテンツの終了
        client.flush();                     // ESP32用 ERR_CONNECTION_RESET対策
        client.stop();                      // クライアントの切断
        lcdPrintVal("TX Bytes",size);       // ファイルサイズを液晶へ表示
        return;                             // 処理の終了・loop()の先頭へ
    }
    if(strncmp(s,"GET /?INT=",10)==0){      // 更新時間の設定命令を受けた時
        update = atoi(&s[10]);              // 受信値を変数updateに代入
    }
    if(strncmp(s,"GET /?SIZE=",11)==0){     // JPEGサイズ設定命令時
        i = atoi(&s[11]);                   // 受信値を変数iに代入
        CamSizeCmd(i);                      // JPEGサイズ設定
    }
    if(!strncmp(s,"GET /favicon.ico",16)){  // Google Chrome対応(追加)
        client.println("HTTP/1.0 404 Not Found");
        client.println();                   // ヘッダの終了
        client.flush();                     // ESP32用 ERR_CONNECTION_RESET対策
        client.stop();                      // クライアントの切断
        return;                             // 処理の終了・loop()の先頭へ
    }
    
    for(i=6;i<strlen(s);i++) if(s[i]==' '||s[i]=='+') s[i]='\0';
    htmlMesg(client,&s[6],WiFi.localIP());  // メッセージ表示
    client.flush();                         // ESP32用 ERR_CONNECTION_RESET対策
    client.stop();                          // クライアント切断
    Serial.println("Sent HTML");            // シリアル出力表示
}
