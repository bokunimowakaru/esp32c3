#!/usr/bin/env python3
# coding: utf-8

# Raspberry Pi によるホームネットワーク i.myMimamoriHome
#                                         Copyright (c) 2016-2022 Wataru KUNINO
#
# IoT人感センサとIoT温度センサを用い、在室中に28℃以上または15℃以下となった時に
# エアコンの運転を開始する。
# また、赤外線リモコンセンサを用い、テレビの操作を検出したときに在室と判断する。
#
# 最新版はこちら：
# https://bokunimo.net/git/myMimamori/blob/master/srv_myhome.py

#【接続図】
#           [IoTボタン]----------------
#                                     ↓
#           [IoT人感センサ]-----------・
#                                     ↓
#           [IoT赤外線リモコン]-------・
#                                     ↓
#           [IoT温度センサ] ------> [本機] ------> エアコン制御
#                                     ↓
#                                     ・ --------> IoTチャイム

#【機器構成】
#
#   本機        IoTボタンが押されたときにIoTチャイムへ鳴音指示
#
#   子機1       IoT Sensor Coreを、以下に設定
#                   Wi-Fi設定   Wi-Fi動作モード : AP＋STA
#                               Wi-Fi STA設定   : ホームゲートウェイのSSIDとPASS
#                   センサ入力  押しボタン      : PingPong
#
#   子機2       IoT Sensor Coreを、以下に設定
#                   Wi-Fi設定   Wi-Fi動作モード : AP＋STA
#                               Wi-Fi STA設定   : ホームゲートウェイのSSIDとPASS
#                   センサ入力  人感センサ      : ON
#                               ★注意：内蔵温度センサはOFFにしておくこと
#
#   子機3       IoT Sensor Coreを、以下に設定
#                   Wi-Fi設定   Wi-Fi動作モード : AP＋STA
#                               Wi-Fi STA設定   : ホームゲートウェイのSSIDとPASS
#                   センサ入力  内蔵温度センサ  : ON
#
#   子機4       IoT チャイム    ./example18_iot_chime_nn.py
#                               ★注意：同一のRaspberry Piでは実験できない(GPIO)

# ★★★ プログラムを終了させたいときは、[Ctrl] + [C]を、2回、実行してください。

#【動作仕様】
# ・エアコンのある部屋（デバイス名に付随する番号が「1～3」）のIoT人感センサが、
#   在室状態を検知し、かつIoT温度センサから取得した温度値が28℃以上だった時に、
#   赤外線リモコン信号をエアコンに送信し、エアコンの運転を開始します。
# ・エアコン運転中、IoT人感センサが10分以上反応しなかった場合に、エアコンを停止
#   します。その後、適温になった場合は、人がエアコンを停止することを想定し、不在
#   時以外はエアコンを停止しない仕様としました。
# ・IoT温度センサから取得した温度値が28℃以上だった時に、通知メール「室温がxx℃
#   になりました」の送信と、チャイム音での警告、エアコンの運転開始を行います。
# ・28℃以上の時は25分間隔、30℃以上の時は5分間隔、32℃以上の時は1分間隔で制御し
#   続けます。居住者が操作を止めたとしても、運転を再開し続ける仕様としました。
# ・在室中、15℃以下の低温時についても、エアコンの運転を開始します。
# ・温度に応じてIoTチャイムを鳴り分けます。
# ・IoT温度センサから（電池切れなどで）2時間以上、受信できなかった時に通知メール
#   「センサの信号がxx時間ありません」を送信します。
# ・IoTボタンが押下されたときに通知メール「ボタンが押されました」を送信し、IoT
#   チャイムでピンポン音を鳴らします。

MAIL_ID   = '************@gmail.com'    ## 要変更 ##    # GMailのアカウント
MAIL_PASS = '************'              ## 要変更 ##    # パスワード
MAILTO    = 'watt@bokunimo.net'         ## 要変更 ##    # メールの宛先

ip_chime  = '192.168.0.XXX'             ## 要変更 ##    # IoTチャイム,IPアドレス
ip_irrc   = '192.168.0.XXX'             ## 要変更 ##    # リモコン送信機アドレス

LINE_TOKEN= 'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx'
                                            # ↑ここにLINEで取得したTOKENを入力
'''
　※LINEに送信したい場合は、お手持ちのLINEアカウントの LINE Notify用のトークンが
　　必要です。以下の手順で取得してください。
    1. https://notify-bot.line.me/ へアクセス
    2. 右上のアカウントメニューから「マイページ」を選択
    3. アクセストークンの発行で「トークンを発行する」を選択
    4. トークン名「raspi」（任意）を入力
    5. 送信先のトークルームを選択する(「1:1でLINE Notifyから通知を受け取る」等)
    6. [発行する]ボタンでトークンが発行される
    7. [コピー]ボタンでクリップボードへコピー
    8. 下記のLINE_TOKENに貼り付け
'''

ROOM      = ['1','2','3']                               # 部屋番号(デバイスSFX)
ROOM_STAY = None                                        # 在室状態
ROOM_RC   = False                                       # 運転フラグ
ALLOWED_TEMP  = [28,15]  #(℃)                          # 冷房／暖房自動運転温度
REED=1                                                  # リード極性ON検出0,OFF1
MON_INTERVAL  = 1 #(分)                                 # 監視処理の実行間隔

# 赤外線リモコンに関する情報を設定してください。
# リモコン方式IR_TYPEには、AEHA(家製協),NEC,SIRC(ソニー)のいずれかを設定し、
# AC_ONとAC_OFFには、エアコンの赤外線リモコンのコードを設定します。
IR_TYPE  = 'AEHA'                                       # 方式 AEHA,NEC,SIRC
AC_ON   = 'AA,5A,CF,10,00,11,20,3F,18,B0,00,F4,B1'      # エアコン電源入コマンド
AC_OFF  = 'AA,5A,CF,10,00,21,20,3F,18,B0,00,F4,81'      # エアコン電源切コマンド
TV_CODE = '48,AA,5A,8F'                                 # テレビのリモコン信号
'''
　TV_CODEの設定方法：
　シャープ製のテレビの場合は'48,AA,5A,8F'、パナソニック製のテレビの場合は、
　'48,02,20,80'を設定してください。先頭の48は、リモコンのコード長（ビット）です。
　ここでは48ビットすなわち6バイトを指定しました。続くAA,5A,8Aや02,20,80がリモコン
　信号の先頭3バイトです。受信した6バイトの先頭3バイトが一致すれば、残る3バイトに
　何が入っていてもテレビのリモコンであると判定します。
'''

# 対応センサ
sensors = ['pir_s','rd_sw','ir_rc','temp.','temp0','humid','press','envir']

# うち温度を含むセンサ
sensors_temp = ['temp.','temp0','humid','press','envir']

# 警告レベル
temp_lv = [ALLOWED_TEMP[0], ALLOWED_TEMP[0]+2 , ALLOWED_TEMP[0]+4 ]

import socket                                           # IP通信用モジュール
import urllib.request                                   # HTTP通信ライブラリ
import datetime                                         # 日時・時刻用ライブラリ
import threading                                        # スレッド用ライブラリ
import smtplib                                          # メール送信用ライブラリ
from email.mime.text import MIMEText                    # メール形式ライブラリ
import sys
sys.path.append('libs/ir_remote')
import raspi_ir

def mimamori(interval):
    t = threading.Timer(interval, mimamori, [interval]) # 遅延起動スレッドを生成
    t.start()                                           # (60秒後に)スレッド起動
    global ROOM_RC, ROOM_STAY, TIME_SENS                # グローバル変数
    time_now = datetime.datetime.now()                  # 現在時刻の取得
    time_sens = TIME_SENS + datetime.timedelta(hours=2) # センサ受信時刻+2時間
    if time_now.minute == 0 and time_sens < time_now:   # 正時かつ2時間受信なし
        s = str(round((time_now - TIME_SENS).seconds / 60 / 60,1))
        msg = 'センサの信号が' + s + '時間ありません'   # メール本文の作成
        mail(MAILTO,'i.myMimamoriPi 警告',msg)          # メール送信関数を実行
    if ROOM_STAY is not None:
        time_stay = ROOM_STAY + datetime.timedelta(seconds=600)  # ＋10分
        if time_stay < time_now:                        # 10分以上、受信なし
            ROOM_STAY = None                            # 不在にリセットする
    if ROOM_STAY is None:
        if ROOM_RC:                                     # 不在なのに運転中のとき
            ROOM_RC = False                             # 運転停止状態に変更
            aircon(False)                               # エアコンの運転停止
#   print('next',t.getName(),'=',time_now +datetime.timedelta(seconds=interval))
                                                        # (スレッド動作確認用)

def chime(level):                                       # チャイム
    if ip_chime  == '192.168.0.XXX':                    # IPアドレス未設定時に
        return                                          # 何もせずに戻る
    if level is None or level < 0 or level > 3:         # 範囲外の値の時に
        return                                          # 何もせずに戻る
    url_s = 'http://' + ip_chime                        # アクセス先
    s = '/?B=' + str(level)                             # レベルを文字列変数sへ
    try:
        urllib.request.urlopen(url_s + s)               # IoTチャイムへ鳴音指示
    except urllib.error.URLError:                       # 例外処理発生時
        print('URLError :',url_s)                       # エラー表示
        # ポート8080へのアクセス用 (下記の5行)
        url_s = 'http://' + ip_chime + ':8080'          # ポートを8080に変更
        try:
            urllib.request.urlopen(url_s + s)           # 再アクセス
        except urllib.error.URLError:                   # 例外処理発生時
            print('URLError :',url_s)                   # エラー表示

def mail(att, subject, text):                           # メール送信用関数

    # LINE送信部
    if LINE_TOKEN != 'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx':
        url_s = 'https://notify-api.line.me/api/notify' # LINE アクセス先
        head = {'Authorization':'Bearer ' + LINE_TOKEN,
                'Content-Type':'application/x-www-form-urlencoded; charset=UTF-8'};
        body = 'message=件名:' + subject + '\n' + text
        print(body.replace('\n',', '))                  # メッセージを表示
        post = urllib.request.Request(url_s, body.encode(), head)
        try:                                            # 例外処理の監視を開始
            urllib.request.urlopen(post)                # HTTPアクセスを実行
        except Exception as e:                          # 例外処理発生時
            print(e,url_s)                              # エラー内容と変数url_s表示

    # メール送信部
    if MAIL_PASS == '************':                     # メール未設定時
        return                                          # 以下を実行しない
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

def aircon(onoff):                                      # エアコン制御
    if onoff:                                           # ON/OFFフラグがTrueのとき
        code = AC_ON.split(',')                         # エアコンをONに
    else:                                               # Falseのとき
        code = AC_OFF.split(',')                        # エアコンをOFFに
    if ip_irrc == '192.168.0.XXX':                      # IPアドレスが未設定のとき
        try:
            print('RC, Conditioner,',code)              # 送信するリモコン信号を表示
            raspiIr.output(code)                        # リモコンコードを送信
        except ValueError as e:                         # 例外処理発生時(アクセス拒否)
            print('ERROR:raspiIr,',e)                   # エラー内容表示
    else:                                               # IPアドレスが設定されている時
        url_s = 'http://' + ip_irrc                     # アクセス先
        s = '/?IR=' + str(len(code) * 8) + ',' + ','.join(code) # 送信信号を変数sへ
        print('RC, Conditioner,',url_s + s)             # 送信するURLを表示
        try:
            urllib.request.urlopen(url_s + s)           # Wi-Fiリモコン送信機に送信
        except urllib.error.URLError:                   # 例外処理発生時
            print('URLError :',url_s)                   # エラー表示

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

TIME_TEMP = TIME_SENS = datetime.datetime.now() - datetime.timedelta(hours=1)
mail(MAILTO,'i.myMimamoriHome','起動しました')          # メール送信

# 赤外線送信の初期化
ir_types = ['AEHA','NEC','SIRC']
if IR_TYPE not in ir_types:
    print('IR_TYPE Error :',IR_TYPE)                    # エラー表示
    exit()                                              # プログラムの終了
if ip_irrc != '192.168.0.XXX':                          # IPアドレスが設定済の時
    url_s = 'http://' + ip_irrc                         # アクセス先
    s = '/?TYPE=' + str(ir_types.index(IR_TYPE))        # リモコン信号方式を設定
    print('RC, Conditioner,',url_s + s)                 # 送信するURLを表示
    try:
        urllib.request.urlopen(url_s + s)               # Wi-Fiリモコン送信機に送信
    except urllib.error.URLError:                       # 例外処理発生時
        print('URLError :',url_s)                       # エラー表示
else:                                                   # IP未設定時
    raspiIr = raspi_ir.RaspiIr(IR_TYPE, out_port=4)     # 赤外線リモコン,Port=4

# 見守りシステムの起動
mimamori(MON_INTERVAL * 60)                             # 関数mimamoriを起動

# UDP受信
print('Listening UDP port', 1024, '...', flush=True)    # ポート番号1024表示
try:
    sock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)# ソケットを作成
    sock.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)    # オプション
    sock.bind(('', 1024))                               # ソケットに接続
except Exception as e:                                  # 例外処理発生時
    print('ERROR, Sock:',e)                             # エラー内容を表示
    exit()                                              # プログラムの終了

while sock:                                             # 永遠に繰り返す
    try:
        udp, udp_from = sock.recvfrom(64)               # UDPパケットを取得
        udp = udp.decode().strip()                      # 文字列に変換
    except KeyboardInterrupt:                           # キー割り込み発生時
        print('\nKeyboardInterrupt')                    # キーボード割り込み表示
        print('Please retype [Ctrl]+[C], 再操作してください')
        exit()                                          # プログラムの終了
    if udp == 'Ping':                                   # 「Ping」に一致する時
        print('Ping',udp_from[0])                       # 取得値を表示
        chime(0)                                        # IoTチャイムを制御
        mail(MAILTO,'i.myMimamoriHome 通知','ボタンが押されました')
        continue                                        # whileへ戻る
    vals = udp.split(',')                               # 「,」で分割
    dev = check_dev_name(vals[0])                       # デバイス名を取得
    if dev is None or len(vals) < 2:                    # 取得なし,又は項目1以下
        continue                                        # whileへ戻る

    now = datetime.datetime.now()                       # 現在時刻を代入
    print(now.strftime('%Y/%m/%d %H:%M')+',', end='')   # 日付を出力
    print(vals[0]+','+udp_from[0]+',', end='')          # デバイス情報を出力
    print(','.join(vals[1:]), end='')                   # センサ値を出力
    bell = 0                                            # 変数bell:IoTチャイム
    acrc = 0                                            # 変数acrc:リモコン制御
    val = get_val(vals[1])                              # 変数valの取得

    # IoT人感センサ用の処理
    if dev[0:5] == 'pir_s':                             # 人感センサの場合
        if dev[6] in ROOM:                              # 自室のセンサだったとき
            if int(val) == 1:                           # 人感検知時
                ROOM_STAY = now                         # 在室状態を更新

    # IoTドアセンサ用の処理(参考)
    if dev[0:5] == 'rd_sw':                             # 人感センサの場合
        if int(val) == REED:                            # 検出極性が一致したとき
            bell = 1                                    # IoTチャイム指示を設定
        else:                                           # 不一致(解除)のとき
            bell = 0                                    # IoTチャイム指示を設定
        if dev[6] in ROOM:                              # 自室のセンサだったとき
            ROOM_STAY = now                             # 在室状態を更新

    # 赤外線リモコンによるテレビ操作を検出
    if dev[0:5] == 'ir_in' or dev[0:5] == 'ir_rc':      # 赤外線リモコンの場合
        if dev[6] in ROOM:                              # 自室センサの時
            if udp.upper().find(TV_CODE) >= 8:          # テレビの時
                ROOM_STAY = now                         # 在室状態を更新

    # 温度センサ用の処理
    level = 0                                           # 温度超過レベル(低温=負
    if dev[0:5] in sensors_temp:                        # 温度値を含むセンサの時
        if val <= ALLOWED_TEMP[1]:                      # 15℃以下のとき
            level = -1                                  # 負のレベル設定
        for temp in temp_lv:                            # 警告レベルを取得
            if val >= temp:                             # 温度が警告レベルを超過
                level = temp_lv.index(temp) + 1         # レベルを代入
        if dev[6] in ROOM:                              # 自室センサ時
            TIME_SENS = now                             # センサ取得時刻を更新
            if (ROOM_STAY is not None) and level != 0:  # 在室中,警告レベル1以上
                acrc = abs(level)                       # 絶対値をacrvへ代入
                bell = acrc                             # IoTチャイム制御を設定
        print(\
            ',stay='+str(ROOM_STAY is not None)+\
            ',bell='+str(bell)+\
            ',temp='+str(val)+'('+str(level)+')'\
        )                                               # 各種の状態を表示
    else:
        print(',stay='+str(ROOM_STAY is not None)+',bell='+str(bell))

    ### 制御 ### IoTチャイム
    if bell > 0:                                        # IoTチャイム制御有効時
        chime(bell)                                     # チャイムを鳴らす

    ### 制御 ### 赤外線リモコン
    if acrc > 0:                                        # エアコン制御有効時
        time_temp = TIME_TEMP + datetime.timedelta(minutes = 5 ** (3 - acrc))
        if time_temp < now:
            msg = '室温が' + str(val) + '℃になりました'
            mail(MAILTO,'i.myMimamoriHome 警告レベル=' + str(level), msg)
            TIME_TEMP = datetime.datetime.now()         # センサ取得時刻を代入
            ROOM_RC = True                              # 運転状態に変更
            aircon(True)                                # 運転開始
'''
実行例
--------------------------------------------------------------------------------
pi@raspberrypi:~ $ ./example39_srv_myhome.py
Mail: watt@bokunimo.net i.myMimamoriHome 起動しました
Listening UDP port 1024 ...
2019/10/22 18:08,temp0_2,192.168.0.4,29,stay=False,bell=0,temp=29.0(1)
2019/10/22 18:08,pir_s_2,192.168.0.3,1, 0,stay=True,bell=0
2019/10/22 18:09,temp0_2,192.168.0.4,29,stay=True,bell=1,temp=29.0(1)
Mail: watt@bokunimo.net i.myMimamoriHome 警告レベル=1 室温が29.0℃になりました
RC, Conditioner, ['AA','5A','CF','10','00','11','20','3F','18','B0','00','F4','B1']
2019/10/22 18:10,temp0_2,192.168.0.4,30,stay=True,bell=2,temp=30.0(2)
Mail: watt@bokunimo.net i.myMimamoriHome 警告レベル=2 室温が30.0℃になりました
RC, Conditioner, ['AA','5A','CF','10','00','11','20','3F','18','B0','00','F4','B1']
--------------------------------------------------------------------------------
'''
