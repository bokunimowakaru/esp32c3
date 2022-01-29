#!/usr/bin/env python3
# coding: utf-8

# Example 38 Raspberry Pi による見守りシステム i.myMimamoriPi
#                                         Copyright (c) 2016-2019 Wataru KUNINO
#
# テレビのリモコン信号と、IoT温度センサを監視し、指定した時間帯に4時間以上、
# テレビ操作が無かったときや、温度が高いときにメールを送信します。

#【接続図】
#           [IoTボタン]----------------
#                                     ↓
#           [IoT赤外線リモコン受信]---・
#                                     ↓
#           [IoT温度センサ] ------> [本機] ------> メール送信

#【機器構成】
#   本機        IoTボタンが押されたときにIoTチャイムへ鳴音指示
#   子機        IoT Sensor Coreを、以下に設定
#                   Wi-Fi設定   Wi-Fi動作モード : AP＋STA
#                               Wi-Fi STA設定   : ホームゲートウェイのSSIDとPASS
#                   センサ入力  内蔵温度センサ  : ON
#                               押しボタン      : PingPong
#                               赤外線RC        : AEHA (SHARP,Panasonic製テレビ)

# ★★★ プログラムを終了させたいときは、[Ctrl] + [C]を、2回、実行してください。

#【動作仕様】
# ・IoT 赤外線リモコン・レシーバが、約4時間以上、テレビのリモコン信号を受信しな
#   かったときに、通知メール「リモコン操作がxx時間ありません」を送信します。
# ・その後、10分毎に確認を行い、リモコン信号を受信するまでメール送信し続けます。
# ・リモコン信号未受信の通知メールは、深夜0時から翌朝7時までは送信しません。
# ・IoT温度センサから取得した温度値が28℃以上だった時に、通知メール「室温がxx℃
#   になりました」を送信します。
# ・30℃以上のときは5分間隔、32℃以上のときは1分間隔で送信し続けます。
# ・IoT温度センサから（電池切れなどで）4時間以上、受信できなかった時に通知メール
#   「センサの信号がxx時間ありません」を送信します。
# ・IoTボタンが押下されたときに通知メール「ボタンが押されました」を送信します。
# ・システムが正常に稼働していることを知らせるために、毎朝、午前9時に、リモコン
#   操作回数を定期報告メールとして送信します。


MAIL_ID   = '************@gmail.com'    ## 要変更 ##    # GMailのアカウント
MAIL_PASS = '************'              ## 要変更 ##    # パスワード
MAILTO    = 'watt@bokunimo.net'         ## 要変更 ##    # メールの宛先
RC_CODE   = '48,aa,5a,8f'               ## 要変更 ##    # テレビのリモコン信号

MONITOR_START =  9  #(時)                               # 監視開始時刻
MON_INTERVAL  = 10  #(分)                               # 監視処理の実行間隔
ALLOWED_TERM  =  4  #(時間)                             # 警報指定時間(22以下)
ALLOWED_TEMP  = 32  #(℃)                               # 警報指定温度
REPORT_TIME   =  9  #(時)                               # 定期報告時刻
sensors = ['ir_in','temp.','temp0','humid','press','envir'] # 対応センサ名
temp_lv = [ALLOWED_TEMP-4, ALLOWED_TEMP-2 , ALLOWED_TEMP ]  # 警告レベル 3段階

import socket                                           # IP通信用モジュール
import urllib.request                                   # HTTP通信ライブラリ
import datetime                                         # 日時・時刻用ライブラリ
import threading                                        # スレッド用ライブラリ
import smtplib                                          # メール送信用ライブラリ
from email.mime.text import MIMEText                    # メール形式ライブラリ

def mimamori(interval):
    t = threading.Timer(interval, mimamori, [interval]) # 遅延起動スレッドを生成
    t.start()                                           # (60秒後に)スレッド起動
    global TIME_REMO, TIME_SENS, REPORT_STAT, COUNT_REMO    # グローバル変数
    time_now = datetime.datetime.now()                  # 現在時刻の取得
    if time_now.hour != REPORT_TIME:                    # 定期報告時刻で無いとき
        REPORT_STAT = 0
    else:
        if REPORT_STAT == 0:                            # 未報告のとき
            REPORT_STAT = 1                             # 報告済みに変更
            s = str(COUNT_REMO)
            COUNT_REMO = 0
            msg = '昨日のリモコン操作は' + s + '回でした。' # メール本文の作成
            mail(MAILTO,'i.myMimamoriPi 定期報告',msg)  # メール送信関数を実行
    time_sens = TIME_SENS + datetime.timedelta(hours=ALLOWED_TERM)
    if time_sens < time_now:                            # センサ送信時刻を超過
        s = str(round((time_now - TIME_SENS).seconds / 60 / 60,1))
        msg = 'センサの信号が' + s + '時間ありません'   # メール本文の作成
        mail(MAILTO,'i.myMimamoriPi 警告',msg)          # メール送信関数を実行
    if time_now.hour < MONITOR_START:                   # AM0時～9時は送信しない
        return
    time_remo = TIME_REMO + datetime.timedelta(hours=ALLOWED_TERM)
    if time_remo < time_now:                            # リモコン送信時刻を超過
        s = str(round((time_now - TIME_REMO).seconds / 60 / 60,1))
        msg = 'リモコン操作が' + s + '時間ありません'   # メール本文の作成
        mail(MAILTO,'i.myMimamoriPi 警告',msg)          # メール送信関数を実行
#   print('next',t.getName(),'=',time_now +datetime.timedelta(seconds=interval))

def mail(att, subject, text):                           # メール送信用関数
    try:
        mime = MIMEText(text.encode(), 'plain', 'utf-8')# TEXTをMIME形式に変換
        mime['From'] = MAIL_ID                          # 送信者を代入
        mime['To'] = att                                # 宛先を代入
        mime['Subject'] = subject                       # 件名を代入
        smtp = smtplib.SMTP('smtp.gmail.com', 587)      # SMTPインスタンス生成
        smtp.starttls()                                 # SSL/TSL暗号化を設定
        smtp.login(MAIL_ID, MAIL_PASS)                  # SMTPサーバへログイン
        smtp.sendmail(MAIL_ID, att, mime.as_string())   # SMTPメール送信
        smtp.close()                                    # 送信終了
        print('Mail:', att, subject, text)              # メールの内容を表示
    except Exception as e:                              # 例外処理発生時
        print('ERROR, Mail:',e)                         # エラー内容を表示
    #   raise e                                         # Exceptionを応答する

def check_dev_name(s):                                  # デバイス名を取得
    if not s.isprintable():                             # 表示可能な文字列で無い
        return None                                     # Noneを応答
    if len(s) != 7 or s[5] != '_':                      # フォーマットが不一致
        return None                                     # Noneを応答
    for sensor in sensors:                              # デバイスリスト内
        if s[0:5] == sensor:                            # センサ名が一致したとき
            return s                                    # デバイス名を応答
    return None                                         # Noneを応答

def get_val(s):                                         # データを数値に変換
    s = s.replace(' ','')                               # 空白文字を削除
    try:                                                # 小数変換の例外監視
        return float(s)                                 # 小数に変換して応答
    except ValueError:                                  # 小数変換失敗時
        return None                                     # Noneを応答

TIME_REMO = TIME_TEMP = TIME_SENS = datetime.datetime.now()
REPORT_STAT = 1
COUNT_REMO = 0
mail(MAILTO,'i.myMimamoriPi','起動しました')            # メール送信

print('Listening UDP port', 1024, '...', flush=True)    # ポート番号1024表示
try:
    sock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)# ソケットを作成
    sock.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)    # オプション
    sock.bind(('', 1024))                               # ソケットに接続
except Exception as e:                                  # 例外処理発生時
    print('ERROR, Sock:',e)                             # エラー内容を表示
    exit()                                              # プログラムの終了
mimamori(MON_INTERVAL * 60)                             # 関数mimamoriを起動

while sock:                                             # 永遠に繰り返す
    try:
        udp, udp_from = sock.recvfrom(64)               # UDPパケットを取得
        udp = udp.decode().strip()                      # 文字列に変換
    except KeyboardInterrupt:                           # キー割り込み発生時
        print('\nKeyboardInterrupt')                    # キーボード割り込み表示
        print('Please retype [Ctrl]+[C], 再操作してください')
        exit()                                          # プログラムの終了
    if udp == 'Ping':                                   # 「Ping」に一致する時
        print('Ping',udp_from[0])              # 取得値を表示
        mail(MAILTO,'i.myMimamoriPi 通知','ボタンが押されました')
        continue                                        # whileへ戻る
    vals = udp.split(',')                               # 「,」で分割
    dev = check_dev_name(vals[0])                       # デバイス名を取得
    if dev is None or len(vals) < 2:                    # 取得なし,又は項目1以下
        continue                                        # whileへ戻る

    now = datetime.datetime.now()                       # 現在時刻を代入
    print(now.strftime('%Y/%m/%d %H:%M')+', ', end='')  # 日付を出力
    # 赤外線リモコン用の処理
    if dev[0:5] == 'ir_in':
        print(vals[0],udp_from[0],',',vals[1:], end='')
        if udp.find(RC_CODE) >= 8:
            TIME_REMO = now                             # リモコン取得時刻を更新
            COUNT_REMO += 1
            print('TV_RC,',COUNT_REMO)                  # テレビリモコン表示
        else:
            print()                                     # 改行
        continue                                        # whileへ戻る

    # 温度センサ用の処理
    val = get_val(vals[1])                              # データ1番目を取得
    level = 0                                           # 温度超過レベル用の変数
    for temp in temp_lv:                                # 警告レベルを取得
        if val >= temp:                                 # 温度が警告レベルを超過
            level = temp_lv.index(temp) + 1             # レベルを代入
    print(vals[0],udp_from[0],',temperature =',val,',level =',level)
    TIME_SENS = now                                     # センサ取得時刻を更新
    if level > 0:                                       # 警告レベル1以上のとき
        time_temp = TIME_TEMP + datetime.timedelta(minutes = 5 ** (3 - level))
        if time_temp < datetime.datetime.now():
            msg = '室温が' + str(val) + '℃になりました'
            mail(MAILTO,'i.myMimamoriPi 警告レベル=' + str(level), msg)
            TIME_TEMP = datetime.datetime.now()         # センサ取得時刻を代入

'''
実行例
--------------------------------------------------------------------------------
pi@raspberrypi:~/iot/learning $ ./example38_srv_mimamori.py
Mail: watt@bokunimo.net i.myMimamoriPi 起動しました
Listening UDP port 1024 ...
2019/10/14 17:39, temp0_2 192.168.0.7 ,temperature = 26.0 ,level = 0
2019/10/14 17:40, temp0_2 192.168.0.7 ,temperature = 26.0 ,level = 0
2019/10/14 17:40, ir_in_2 192.168.0.7 , ['48', 'aa', '5a', '8f', '12', '16', 'd1']TV_RC, 1

--------------------------------------------------------------------------------
'''
