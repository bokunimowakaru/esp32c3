/*******************************************************************************
Example 9 : ESP32C3 Wi-Fi コンシェルジェ アナウンス担当（音声合成出力）
AquesTalkを使った音声合成でユーザへ気づきを通知することが可能なIoT機器です。

    AquesTalk接続用
    TXD -> AquesTalk側 RXD端子(2番ピン)

                                          Copyright (c) 2016-2019 Wataru KUNINO

（参考文献）AquesTalk pico：https://www.a-quest.com/products/aquestalk_pico.html
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WebServer.h>                      // HTTPサーバ用ライブラリ

#define PIN_LED_RGB 2                       // IO2 に WS2812を接続(m5stamp)
// #define PIN_LED_RGB 8                    // IO8 に WS2812を接続(DevKitM)
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define PORT 1024                           // 受信ポート番号

WebServer server(80);                       // Webサーバ(ポート80=HTTP)定義

void handleRoot(){
    char talk[97] = "de-ta'o'nyu-ryo_kushiteku'dasai."; // 音声出力用の文字列
    Serial.println("Connected");            // 接続されたことをシリアル出力表示
    if(server.hasArg("TEXT")){              // 引数TEXTが含まれていた時
        String rx = server.arg("TEXT");     // 引数TEXTの値を取得し変数rxへ代入
        rx.toCharArray(talk,97);
        trUri2txt(talk);                    // URLエンコードの変換処理
    }
    if(server.hasArg("VAL")){               // 引数VALが含まれていた時
        int i = server.arg("VAL").toInt();  // 引数VALの値を取得し変数rxへ代入
        snprintf(talk,96,"su'-tiwa <NUMK VAL=%d>desu.",i);
    }
    if(strlen(talk) > 0){                   // 文字列が代入されていた場合、
        led(40,0,0);                        // (WS2812)LEDを赤色で点灯
        Serial.print("\r$");                // ブレークコマンドを出力する
        delay(100);                         // 待ち時間処理
        Serial.print(talk);                 // 受信文字データを音声出力
        Serial.print('\r');                 // 改行コード（CR）を出力する
        led(0,20,0);                        // (WS2812)LEDを緑色で点灯
    }
    String tx = getHtml(talk);              // HTMLコンテンツを取得
    server.send(200, "text/html", tx);      // HTMLコンテンツを送信
}

void setup(){                               // 起動時に一度だけ実行する関数
    led_setup(PIN_LED_RGB);                 // WS2812の初期設定(ポート設定)
    Serial.begin(9600);                     // AquesTalkとの通信ポート
    Serial.print("\r$");                    // ブレークコマンドを出力する
    delay(100);                             // 待ち時間処理
    Serial.print("$?kon'nnichi/wa.\r");     // 音声「こんにちわ」を出力する
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        led((millis()/50) % 10);            // (WS2812)LEDの点滅
        delay(50);                          // 待ち時間処理
    }
    Serial.print("<NUM VAL=");              // 数字読み上げ用タグ出力
    Serial.print(WiFi.localIP());           // IPアドレスを読み上げる
    Serial.print(">.\r");                   // タグの終了を出力する
    server.on("/", handleRoot);             // HTTP接続時のコールバック先を設定
    server.begin();                         // Web サーバを起動する
    led(0,20,0);                            // (WS2812)LEDを緑色で点灯
}

void loop(){                                // 繰り返し実行する関数
    server.handleClient();                  // クライアントからWebサーバ呼出
}
