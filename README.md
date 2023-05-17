# esp32c3
Arduino Code Examples of I/O controled Wireless Communications for ESP32-C3 and M5Stamp C3/C3U 

![ESP32-C3チップを開発したEspressif System純正の開発ボードESP32-C3-DevKitM-1](https://bokunimo.net/blog/wp-content/uploads/2021/06/DSC_0404.jpg)

## 本コンテンツの最新版とダウンロード方法  

- 最新版のダウンロード(ZIP形式)  
    [https://bokunimo.net/git/esp32c3/archive/refs/heads/master.zip](https://bokunimo.net/git/esp32c3/archive/refs/heads/master.zip)    
    
- ダウンロード方法(gitコマンド)  
    git clone https://bokunimo.net/git/esp32c3/  

## 解説書の無料ダウンロード

本レポジトリ内のプログラムの解説書は、下記から無料でダウンロードできます。

- 解説書(PDF版)ダウンロード  
    [https://bokunimo.net/esp32/esp32c3](https://bokunimo.net/esp32/esp32c3)  

## 主なフォルダ名、プログラム名

本レポジトリに収録した主なプログラムのフォルダ名、ファイル名の一覧を示します。  

|フォルダ名 プログラム名    |内容                                                                               |
|---------------------------|-----------------------------------------------------------------------------------|
|learning/ex00_io           |IO実験用ボードの動作確認用プログラム                                               |
|learning/ex01_led_io(※)   |LED制御用プログラム。HTTPサーバ機能によりブラウザから制御可能                      |
|learning/ex02_sw_io(※)    |押しボタンの送信プログラム。ex01_ledのLEDの制御やLINEへの送信が可能                |
|learning/ex03_lum          |照度センサの送信プログラム。照度値をクラウド(Ambient)に送信しグラフ化が可能        |
|learning/ex04_lcd          |小型液晶への表示プログラム。ex02、03、05の送信データの表示が可能                   |
|learning/ex05_hum          |温度＋湿度センサの送信プログラム。家じゅうの部屋に設置すれば居住環境の監視が可能   |
|learning/ex06_pir          |人感センサ・ユニット（PIR Motion Sensor）を使ったWi-Fi人感センサ用プログラム       |
|learning/ex07_ir_in        |赤外線リモコン・ユニット（IR Unit）でリモコンコードを取得するプログラム            |
|learning/ex08_ir_out       |赤外線リモコン・ユニット（IR Unit）を使ったWi-Fi赤外線・リモコン用プログラム       |
|learning/ex09_talk         |Wi-Fiコンシェルジェ［音声アナウンス担当］音声合成 AquesTalk Pico LSI ATP3012用     |
|learning/ex10_cam          |Wi-Fiコンシェルジェ［カメラ担当］Grove - Serial Camera Kit用                       |
|tools/udp_logger.py        |PCやラズベリー・パイ上でセンサ値を受信するPythonプログラム。ex02、03、05に対応     |
|tools/udp_monitor_chart.py |センサ値の棒グラフ化、CSV保存、履歴表示などに対応したPythonプログラム              |
|tools/srv_myhome.py        |自分だけのMyホーム・オートメンション・システム用サンプル・プログラム               |
|benchmark                  |M5Stamp C3/C3U内蔵のESP32-C3マイコン処理能力を測定するのに使用したプログラム       |
|deepsleep                  |乾電池による長期間駆動が可能なプログラムex02、03、04に使用したdeep sleep機能       |
|led_ws2812                 |M5Stamp C3/C3U内蔵のLEDを制御するためのプログラム                                  |

※ 「ex01_led_io」と「ex02_sw_io」はGPIO制御版、「ex01_led」と「ex02_sw」はM5Stamp C3/C3Uに実装されたLEDやボタン対応版、  

## learning  
M5Stamp C3, M5Stamp C3U を使った 無線IO制御プログラミング学習用コンテンツです。  

## benchmark
詳細＝ [https://bokunimo.net/blog/esp/1491/](https://bokunimo.net/blog/esp/1491/)

## led_ws2812
詳細＝ [https://bokunimo.net/blog/esp/1522/](https://bokunimo.net/blog/esp/1522/)

## deepsleep
詳細＝ [https://bokunimo.net/blog/esp/1551/](https://bokunimo.net/blog/esp/1551/)

## Arduino IDE 用の ESP32 開発環境のセットアップ  

1. Arduino IDE (https://www.arduino.cc/en/software/) をインストールする。  
2. Arduino IDE を起動し、[ファイル]メニュー内の[環境設定]を開き、「追加のボードマネージャのURL」の欄に下記の「安定板」を追加する。  

    安定板  
    - https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json  

    開発途上版  
    - https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json  

    参考文献  
    - [https://github.com/espressif/arduino-esp32](https://github.com/espressif/arduino-esp32) (最新情報)  
    - [https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html) (情報が古い場合があるので注意)  

3. [ツール]メニュー内の[ボード]からボードマネージャを開き、検索窓に「esp32」を入力後、esp32 by Espressif Systems をインストールする。  

4. [ツール]メニュー内の[ボード]で ESP32C3 DEev Module を選択する。  

by bokunimo.net(https://bokunimo.net/)  

- ボクにもわかる電子工作のブログ  
  [https://bokuniomo.net/blog/](https://bokuniomo.net/blog/)  
- ボクにもわかる ESP32 のブログ  
  [https://bokunimo.net/blog/category/esp/](https://bokunimo.net/blog/menu/esp/)  
- ボクにもわかる ESP32  
  [https://bokunimo.net/esp32/](https://bokunimo.net/esp32/)  

----------------------------------------------------------------

## GitHub Pages (This Document)
* [https://git.bokunimo.com/esp32c3/](https://git.bokunimo.com/esp32c3/)  

----------------------------------------------------------------

# git.bokunimo.com GitHub Pages site
[http://git.bokunimo.com/](http://git.bokunimo.com/)  

----------------------------------------------------------------
