# esp32c3
Example code for ESP32-C3, written for Arduino IDE

## 本コンテンツの最新版とダウンロード方法  

    最新版の保存先  
    - https://bokunuimo.net/git/esp32c3/
    
    ダウンロード方法(GitHubから)
    - git clone https://bokunuimo.net/git/esp32c3/

## learning  
M5Stamp C3, M5Stamp C3U を使った 無線IO制御プログラミング学習用コンテンツです。  

## benchmark
詳細＝ https://bokunimo.net/blog/esp/1491/

## led_ws2812
詳細＝ https://bokunimo.net/blog/esp/1522/

## deepsleep
詳細＝ https://bokunimo.net/blog/esp/1551/

## Arduino IDE 用の ESP32 開発環境のセットアップ  

1. Arduino IDE (https://www.arduino.cc/en/software/) をインストールする。  
2. Arduino IDE を起動し、[ファイル]メニュー内の[環境設定]を開き、「追加のボードマネージャのURL」の欄に下記の「安定板」を追加する。  

    安定板  
    - https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json  

    開発途上版  
    - https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json  

    参考文献  
    - https://github.com/espressif/arduino-esp32 (最新情報)  
    - https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html (情報が古い場合があるので注意)  

3. [ツール]メニュー内の[ボード]からボードマネージャを開き、検索窓に「esp32」を入力後、esp32 by Espressif Systems をインストールする。  

4. [ツール]メニュー内の[ボード]で ESP32C3 DEev Module を選択する。  

by bokunimo.net(https://bokunimo.net/)  
- ブログ (https://bokuniomo.net/blog/)  
- カテゴリESP (https://bokunimo.net/blog/category/esp/)  
