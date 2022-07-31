#!/usr/bin/env python3
# coding: utf-8

################################################################################
# UDPで受信したIoTセンサ機器の値を棒グラフで表示します。
# （Webブラウザで http://127.0.0.1:8080 にアクセスするとグラフ表示されます）
#
# 最新版：
# https://bokunimo.net/git/udp/blob/master/udp_monitor/udp_monitor_chart.py
#
#                                          Copyright (c) 2021-2022 Wataru KUNINO
################################################################################

# 初期設定
UDP_PORT = 1024             # UDP待ち受けポート番号(デフォルトは1024)
HTTP_PORT = 80              # HTTP待ち受けポート番号(デフォルトは80->失敗時8080)
SAVE_CSV = True             # CSVファイルの保存(True:保存,False:保存しない)
DEV_CHECK = False           # 未登録デバイス保存(True:破棄,False:UNKNOWNで保存)
ELEVATION = 0               # 標高(m) 気圧値の補正用
HIST_BUF_N = 10             # 1センサ値あたりの履歴保持数
DEVICE_MAX = 50             # 最大デバイス数(管理台数)

# 補正用(表示のみ・保存データは補正されない)
OFFSET_VALUE = {\
    'temp._1':(1, 1.0, 0.0),\
    'temp._2':(1, 1.0, 0.0),\
    'temp._3':(1, 1.0, 0.0),\
}#   device   col A    B       col:列番号1～,  corrected = A * value + B

# センサ機器用登録デバイス（UDPペイロードの先頭5文字）
sensors = [\
    'temp0','hall0','adcnv','btn_s','pir_s','illum',\
    'temp.','humid','press','envir','accem','rd_sw',\
    'press','e_co2','janke',\
    'actap','awsin','count','esp32','ident','medal',\
    'meter','ocean','river','tftpc','timer','voice',\
    'xb_ac','xb_ct','xb_ev','xb_sw','xbbat','xbbel',\
    'xbgas','xbhum','xblcd','xbled','xbprs','xbrad',\
    'xbsen'\
]

# センサ機器用CSV形式データの項目（数値データ）
csvs = {\
    'btn_s':[('Ringing','')],\
    'pir_s':[('Wake up Switch',''),('PIR Switch','')],\
    'illum':[('Illuminance','lx')],\
    'rd_sw':[('Wake up Switch',''),('Reed Switch','')],\
    'temp0':[('Temperature','deg C')],\
    'temp.':[('Temperature','deg C')],\
    'ocean':[('Temperature','deg C'),('RSSI','dBm')],\
    'humid':[('Temperature','deg C'),('Humidity','%')],\
    'press':[('Temperature','deg C'),('Pressure','hPa')],\
    'envir':[('Temperature','deg C'),('Humidity','%'),('Pressure','hPa')],\
    'e_co2':[('Temperature','deg C'),('Humidity','%'),('Pressure','hPa'),('CO2','ppm'),('TVOC','ppb'),('Counter','')],\
    'janke':[('Janken',''),('Fingers','')],\
    #'accem':[('Accelerometer X','g'),('Accelerometer Y','g'),('Accelerometer Z','g')],\
    'accem':[('Accelerometer X','m/s2'),('Accelerometer Y','m/s2'),('Accelerometer Z','m/s2')],\
    'actap':[('Power','W'),('Cumulative','Wh'),('Time','Seconds')],\
    'count':[('Counter','')],\
    'meter':[('Power','W'),('Cumulative','Wh'),('Time','Seconds')],\
    'awsin':[('Participants',''),('Cumulative','')],\
    'xb_ac':[('Usage Time','h'),('Consumption','kWh'),('Prev. Usage Time','h'),('Consumption','kWh')],\
    'xb_ct':[('Power','W')],\
    'xb_ev':[('Illuminance','lx'),('Temperature','deg C'),('Humidity','%')],\
    'xb_sw':[('Reed Switch','')],\
    'xbbel':[('Ringing','')],\
    'xbgas':[('CO','ppm'),('CH4','ppm')],\
    'xbhum':[('Illuminance','lx'),('Temperature','deg C'),('Humidity','%')],\
    'xblcd':[('Illuminance','lx'),('Temperature','deg C')],\
    'xbled':[('Illuminance','lx'),('Temperature','deg C')],\
    'xbprs':[('Temperature','deg C'),('Pressure','hPa')],\
    'xbrad':[('Radiation','uSievert'),('Temperature','deg C'),('Voltage','V')],\
    'xbsen':[('Illuminance','lx'),('Temperature','deg C'),('Low Battery','')]\
}

# データ範囲
csvs_range = {\
    ('Wake up Switch',''):      (0,1),\
    ('PIR Switch',''):          (0,1),\
    ('Reed Switch',''):         (0,1),\
    ('Temperature','deg C'):    (0,40),\
    ('RSSI','dBm'):             (-100,0),\
    ('Humidity','%'):           (0,100),\
    ('Pressure','hPa'):         (1013.25 - 33.25, 1013.25 + 33.25),\
    ('CO','ppm'):               (0,2000),\
    ('CO2','ppm'):              (0,2000),\
    ('CH4','ppm'):              (0,2000),\
    ('TVOC','ppb'):             (0,5000),\
    ('Counter',''):             (0,30),\
    ('Fingers',''):             (0,5),\
    ('Accelerometer X','m/s2'): (-9.8,9.8),\
    ('Accelerometer Y','m/s2'): (-9.8,9.8),\
    ('Accelerometer Z','m/s2'): (-9.8,9.8),\
    ('Accelerometer X','g'):    (-1,1),\
    ('Accelerometer Y','g'):    (-1,1),\
    ('Accelerometer Z','g'):    (-1,1),\
    ('Power','W'):              (0,3000),\
    ('Cumulative','Wh'):        (0,3000),\
    ('Consumption','kWh'):      (0,3),\
    ('Time','Seconds'):         (0,3600),\
    ('Time','Hours'):           (0,8760),\
    ('Usage Time','h'):         (0,24),\
    ('Prev. Usage Time','h'):   (0,24),\
    ('Participants',''):        (0,100),\
    ('Cumulative',''):          (0,100000),\
    ('Illuminance','lx'):       (0,1000),\
    ('Ringing',''):             (0,1),\
    ('Radiation','uSievert'):   (0.04,0.23),\
    ('Voltage','V'):            (0,5),\
    ('Low Battery',''):         (0,1)\
}

# センサ機器以外（文字データ入り）の登録デバイス
notifyers = [\
    'adash','atalk','cam_a','ir_in','janke','sound',\
    'xb_ir','xbidt'\
]

# 特定文字列
pingpongs = [
    'Ping','Pong','Emergency','Reset'\
]

devices = list()
dev_vals = dict()
dev_date = dict()
http_port = HTTP_PORT

import os
import sys
import socket
import datetime
from wsgiref.simple_server import make_server       # WSGIサーバ
from getpass import getuser                         # ユーザ取得を組み込む
from time import time                               # 時間取得を組み込む
from time import sleep                              # スリープ機能を組み込む
import threading                                    # スレッド管理を組み込む

def get_dev_name(s):                                    # デバイス名を取得
    if s.strip() in pingpongs:                          # Ping または Pong
        return s.strip()
    if not s[0:8].isprintable():
        return None                                     # Noneを応答
    if s[5] == '_' and s[6].isdecimal() and s[7] == ',': # 形式が一致する時
        if s[0:5] in sensors:                           # センサリストの照合
            return s[0:7]                               # デバイス名を応答
        if s[0:5] in notifyers:                         # センサリストの照合
            return s[0:7]                               # デバイス名を応答
    return None                                         # Noneを応答

def get_val(s):                                         # データを数値に変換
    s = s.replace(' ','')                               # 空白文字を削除
    try:                                                # 小数変換の例外監視
        val = float(s)                                  # 小数値に変換
    except ValueError:                                  # 小数変換失敗時
        return None                                     # Noneを応答
    if float(int(val)) == val:                          # valが整数のとき
        return int(val)                                 # 整数値を応答
    else:
        return val                                      # 小数値を応答

def calc_press_h0(temp,press):
    print('calc_press_h0',temp,press,end=' -> ')
    temp += 273.15 + 0.0065 * ELEVATION                 # 海面(標高)温度
    press /= (1.0 - 0.0065 * ELEVATION / temp) ** 5.257 # 海面(標高)気圧
    press = round(press,1)
    print(press)
    return press

def save(filename, data):
    if SAVE_CSV == False:
        return
    try:
        fp = open(filename, mode='a')                   # 書込用ファイルを開く
    except Exception as e:                              # 例外処理発生時
        print(e)                                        # エラー内容を表示
    fp.write(data + '\n')                               # dataをファイルへ
    fp.close()                                          # ファイルを閉じる

def barChartHtml(colmun, range, val, color='lightgreen'):    # 棒グラフHTMLを作成する関数
    unit = ''
    if len(colmun) > 0:
        if colmun == 'deg C':
            unit = ' ℃'
        elif colmun == 'uSievert':
            unit = ' μSv'
        elif colmun == 'm/s2':
            unit = ' m/s²'
        else:
            unit = ' ' + colmun
    html = '<td align="right">' + str(val) + unit + '</td>\n' # 変数valの値を表示
    min = range[0]
    max = range[1]
    i= round(200 * (val - min) / (max - min))       # 棒グラフの長さを計算
    if val - min <= (max - min) * 0.2:              # 20％以下のとき
        color = 'lightblue'                         # 棒グラフの色を青に
    if val - min >= (max - min) * 0.8:              # 80％以上のとき
        color = 'lightpink'                         # 棒グラフの色をピンクに
    if val > max or val < min:                      # 最大値or最小値を超えた時
        color = 'red'                               # 棒グラフの色を赤に
        i = 200                                     # グラフ長を200ポイントに
    html += '<td align ="right" valign="bottom"><div style="font-size:xx-small;">' + str(min) + '</div></td>\n'
    html += '<td width=200><div style="background-color: ' + color
    html += '; width: ' + str(i) + 'px">&nbsp;</div></td>\n'
    html += '<td valign="bottom"><div style="font-size:xx-small;">' + str(max) + '</div></td>\n'
    return html                                     # HTMLデータを返却

def wsgi_app(environ, start_response):              # HTTPアクセス受信時の処理
    path  = environ.get('PATH_INFO')                # リクエスト先のパスを代入
    # print('debug path:',path)                     ##確認用
    if not path.isprintable():
        start_response('404 Not Found',[])          # 404エラー設定
        return ['404 Not Found'.encode()]           # 応答メッセージ(404)を返却
    if path != '/' and (len(path)!=16 or path[1:5] != 'log_' or path[12:16] != '.csv'):
        start_response('404 Not Found',[])          # 404エラー設定
        return ['404 Not Found'.encode()]           # 応答メッセージ(404)を返却

    queries  = environ.get('QUERY_STRING')
    if not queries.isprintable() or len(queries) > 256:
        start_response('404 Not Found',[])          # 404エラー設定
        return ['404 Not Found'.encode()]           # 応答メッセージ(404)を返却
    if environ.get('REQUEST_METHOD') != 'GET':
        return ['404 Not Found'.encode()]           # 応答メッセージ(404)を返却
    print('debug queries:',queries)                 ##確認用
    queries  = queries.lower().split('&')
    # print('debug queries:',queries)               ## 確認用
    if path[5:10] in sensors:
        filename = 'log_' + path[5:12] + '.csv'
        try:
            fp = open(filename, 'rb')
            start_response('200 OK', [('Content-type', 'application/force-download')])
            return[fp.read()]
        except Exception as e:
            start_response('404 Not Found',[])      # 404エラー設定
            return ['404 Not Found'.encode()]       # 応答メッセージ(404)を返却
    if path != '/':                                 # パスがルート以外のとき
        start_response('404 Not Found',[])          # 404エラー設定
        return ['404 Not Found'.encode()]           # 応答メッセージ(404)を返却

    html = '<html>\n<head>\n'                       # HTMLコンテンツを作成
    html += '<meta http-equiv="refresh" content="10;">\n'   # 自動再読み込み
    html += '</head>\n<body>\n'                     # 以下は本文
    html += '<h1>UDPセンサ用モニタ (<a href="/">'\
          + str(len(devices));
    if len(devices) == 1:
        html += ' device</a>)</h1>\n'
    else:
        html += ' devices</a>)</h1>\n'

    sort_col = 'devices'
    filter_dev = list()
    filter_item = list()
    hist = 0
    for query in queries:
        if query == '' or query == 'devices':
            sort_col = 'devices'
        if query == 'items':
            sort_col = 'items'
        if query.startswith('device='):
            filter_dev.append(query[7:12])
        if query.startswith('item='):
            filter_item.append(query[5:])
        if query.startswith('hist='):
            filter_dev.append(query[5:12])
            hist = 1

    html += 'Filter :'
    if len(filter_dev) > 0:
        html += ' <a href="/">device</a> = ' + str(filter_dev)
    if len(filter_item) > 0:
        html += ' <a href="/?items">item</a> = ' + str(filter_item)
    if len(filter_dev) == 0 and len(filter_item) == 0:
        html += ' None'

    if sort_col == 'devices':
        html += ', <a href="/?items">Order</a> : '
    else:
        html += ', <a href="/?devices">Order</a> : '
    html += sort_col + '<br>\n'

    html += '<table border=1>\n'                    # 作表を開始
    html += '<tr><th><a href="?devices">デバイス名</a></th>'
    html += '<th><a href="?items">項目</a></th><th>日 時:分:秒</th><th>値</th>'
    html += '<th colspan = 3>グラフ</th></tr>\n'    # 「グラフ」を表示
    col_dict = dict()
    for dev in sorted(devices):
        if (len(filter_dev) > 0) and (dev[0:5] not in filter_dev) and (dev[0:7] not in filter_dev):
            continue
        if dev[0:5] in sensors:
            colmuns = csvs.get(dev[0:5])
            if colmuns is None:
                print('[ERROR] founds no devices on csvs dictionary; dev =',dev[0:5])
                continue
            # i_max = min(len(colmuns), len(dev_vals[dev][-1]))
            i_max = len(colmuns)
            for j in range(len(dev_vals[dev])):
                i_max = min(i_max, len(dev_vals[dev][j]))
            if dev[0:5] == 'actap':  # 数が多いので電力のみを表示する
                i_max = 1
            for i in range(i_max):
                colmun = csvs[dev[0:5]][i]
                minmax = csvs_range.get(colmun)
                if minmax is None:
                    minmax = (0.0, 1.0)
                val = dev_vals[dev][-1][i]
                if val is None:
                    val = 0
                if sort_col == 'devices':
                    i_max_hist = i_max
                    if hist > 0:
                        hist = len(dev_vals[dev])
                        i_max_hist *= hist
                    if i == 0:
                        html += '<tr><td rowspan = ' + str(i_max_hist) + '>'
                        if hist > 0:
                              filename = 'log_' + dev[0:7] + '.csv'
                              html += dev[0:7] + '<br>[<a href="' + filename + '">csv</a>]</td>'
                        elif dev[0:5] in filter_dev:
                              html += '<a href="?hist=' + dev[0:7] + '">' + dev[0:7] + '</td>'
                        else:
                              html += '<a href="?device=' + dev[0:5] + '">'\
                              + dev[0:5] + '</a> <a href="?hist=' + dev[0:7] + '">' + dev[6] + '</td>'
                    else:
                        html += '<tr>'

                    if hist > 0:
                        html += '<td rowspan = ' + str(hist) + '><a href="?items&item=' + colmun[0] + '">'\
                              + colmun[0] + '</a></td>\n'
                        for j in range(hist):
                            if j > 0:
                                html += '<tr>'
                            val = dev_vals[dev][j][i]
                            if val is None:
                                val = 0
                            html += '<td align ="center">' + dev_date[dev][j].strftime('%d %H:%M:%S') + '</td>'
                            html += barChartHtml(colmun[1], minmax, val)   # 棒グラフ化
                            html += '</tr>\n'
                    else:
                        html += '<td><a href="?items&item=' + colmun[0] + '">'\
                              + colmun[0] + '</a></td>\n'
                        html += '<td align ="center">' + dev_date[dev][-1].strftime('%d %H:%M:%S') + '</td>'
                        html += barChartHtml(colmun[1], minmax, val)   # 棒グラフ化

                elif sort_col == 'items':
                    if len(filter_item) == 0 or colmun[0].lower() in filter_item:
                        if colmun not in col_dict:
                            col_dict[colmun] = list()
                        col_dict[colmun].append(dev)
    # print('debug col_dict:',col_dict) ##確認用
    if len(col_dict) > 0:
        for colmun in sorted(col_dict):
            # print('debug colmun:',colmun) ##確認用
            j = 0
            for dev in col_dict[colmun]:
                minmax = csvs_range.get(colmun)
                if minmax is None:
                    minmax = (0.0, 1.0)
                i = csvs[dev[0:5]].index(colmun)
                val = dev_vals[dev][-1][i]
                if val is None:
                    val = 0
                html += '<tr><td><a href="?device=' + dev[0:5] + '">'\
                      + dev[0:5] + '</a> <a href="?hist=' + dev[0:7] + '">' + dev[6] + '</td>'
                if j == 0:
                    html += '<td rowspan = ' + str(len(col_dict[colmun])) + '>'\
                         + '<a href="?items&item=' + colmun[0] + '">'\
                         + colmun[0] + '</a></td>\n'
                # print('debug barChartHtml:', minmax, val) ##確認用
                html += '<td align ="center">' + dev_date[dev][-1].strftime('%d %H:%M:%S') + '</td>'
                html += barChartHtml(colmun[1], minmax, val)   # 棒グラフ化
                j += 1

    html += '<tr><td colspan=7 align=right>'
    html += '<div><font size=2>Usage: http://127.0.0.1'
    if http_port != 80:
        html += ':' + str(http_port)
    html += '/?{devices|items}[&device=name][&item=name][&hist=device_name]</font></div>\n'
    html += '<div>Copyright (c) 2021 <a href="https://bokunimo.net">Wataru KUNINO</a></div>\n'
    html += '</tr>\n</table>\n'                     # 表の終了
    html += '</body>\n</html>\n'                    # htmlの終了
    start_response('200 OK', [('Content-type', 'text/html; charset=utf-8')])
    return [html.encode('utf-8')]                   # 応答メッセージを返却

def httpd(port = HTTP_PORT):
    try:
        htserv = make_server('', port, wsgi_app)    # TCPポート80でHTTPサーバ実体化
    except PermissionError:                         # 例外処理発生時(アクセス拒否)
        port += 8000
        if port > 65535:
            port = 8080
        htserv = make_server('', port, wsgi_app)    # ポート8080でHTTPサーバ実体化
    global http_port
    http_port = port
    print('HTTP port', http_port)
    htserv.serve_forever()                          # HTTPサーバを起動

buf_n= 128                                          # 受信バッファ容量(バイト)
argc = len(sys.argv)                                # 引数の数をargcへ代入
print('UDP Logger (usage: '+sys.argv[0]+' port)')   # タイトル表示
if argc >= 2:                                       # 入力パラメータ数の確認
    port = int(sys.argv[1])                         # ポート番号を設定
    if port < 1 or port > 65535:                    # ポート1未満or65535超の時
        port = UDP_PORT                             # UDPポート番号を1024に
else:
    port = UDP_PORT
sock = None
thread = None

while True:
    if not thread or not thread.is_alive():             # HTTPDが動作していないとき
        print('Starting httpd', http_port, '...')       # ポート番号表示
        thread = threading.Thread(target=httpd, daemon=True) # スレッドhttpdの実体化
        thread.start()                                  # httpdの起動
    if not sock:
        print('Listening UDP port', port, '...')        # ポート番号表示
        try:
            sock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)# ソケットを作成
            sock.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)    # オプション
            sock.bind(('', port))                       # ソケットに接続
        except Exception as e:                          # 例外処理発生時
            print(e)                                    # エラー内容を表示
            delay(30)                                   # 連続再接続防止用の待ち時間
            continue                                    # 再接続
    udp, udp_from = sock.recvfrom(buf_n)                # UDPパケットを取得
    try:
        udp = udp.decode()                              # UDPデータを文字列に変換
    except Exception as e:                              # 例外処理発生時
        print(e)                                        # エラー内容を表示
        continue                                        # whileの先頭に戻る
    if len(udp) <= 4:
        continue
    dev = get_dev_name(udp)                             # デバイス名を取得
    if dev is None:                                     # 不適合
        if DEV_CHECK:                                   # デバイス選別モード時
            continue                                    # whileに戻る
        dev = 'UNKNOWN'                                 # 不明デバイス

    vals = list()
    if len(udp) > 8:
        vals = udp[8:].strip().split(',')               # 「,」で分割
    date = datetime.datetime.today()                    # 日付を取得
    date_s = date.strftime('%Y/%m/%d %H:%M')            # 日付を文字列に変更
    s = ''                                              # 文字列変数
    if dev[0:5] in sensors:
        for val in vals:                                # データ回数の繰り返し
            i = get_val(val)                            # データを取得
            s += ', '                                   # 「,」を追加
            if i is not None:                           # データがある時
                s += str(i)                             # データを変数sに追加
    elif dev in pingpongs:
        print('Ping/Pong Device,',dev,'-> btn_s_1')
        if dev == 'Ping':
            vals = ['1']
            s = ', 1'
        elif dev == 'Pong':
            vals = ['0']
            s = ', 0'
        else:
            vals = ['-1']
            s = ', -1'
        dev = 'btn_s_1'
    else:
        s = ', '                                        # 文字列変数
        for c in udp:                                   # UDPパケット内
            if ord(c) >= ord(' ') and ord(c) <= ord('~'):   # 表示可能文字
                s += c                                  # 文字列sへ追加
    filename = 'log_' + dev + '.csv'                    # ファイル名を作成
    if dev not in devices:
        print('NEW Device,',dev)
        if len(devices) > DEVICE_MAX:                   # 管理可能台数を超過
            print('over the limit, DEVICE_MAX,',devices)
            continue                                    # whileに戻る
        devices.append(dev)
        # print(sorted(devices))
        if SAVE_CSV and not os.path.exists(filename):
            fp = open(filename, mode='w')               # 書込用ファイルを開く
            fp.write('YYYY/MM/dd hh:mm, IP Address')    # CSV様式
            column = csvs.get(dev[0:5])
            if column is not None:
                for col in column:
                    if col[1] == '':
                        fp.write(', ' + col[0])
                    else:
                        fp.write(', ' + col[0] + '(' + col[1] + ')')
            fp.write('\n')
            fp.close()                                  # ファイルを閉じる
    print(date_s + ', ' + dev + ', ' + udp_from[0], end = '')  # 日付,送信元を表示
    if SAVE_CSV:
        print(s, '-> ' + filename, flush=True)          # 受信データを表示
        save(filename, date_s + ', ' + udp_from[0] + s)   # ファイルに保存
    else:
        print(s, flush=True)                            # 受信データを表示

    # 数値データの変数保持(HTML表示用)
    if dev[0:5] in sensors:                             # センサ(数値データ)のとき
            # (len(vals)>0だと値なし時に辞書追加されないのでsensorsかどうかで判定)
        if dev not in dev_vals:
            dev_vals[dev] = list()                      # 数値データを保持
            dev_date[dev] = list()                      # 時刻データを保持
        try:                                                # 小数変換の例外監視
            if dev[0:5] == 'press':
                vals[1] = str(calc_press_h0(get_val(vals[0]),get_val(vals[1])))
            if dev[0:5] == 'envir' or dev[0:5] == 'e_co2':
                vals[2] = str(calc_press_h0(get_val(vals[0]),get_val(vals[2])))
            if dev[0:7] in OFFSET_VALUE:
                offset = OFFSET_VALUE[dev[0:7]]
                vals[offset[0]-1] = str(offset[1] * get_val(vals[offset[0]-1]) + offset[2])
        except IndexError:
            print('ERROR: list index out of range')
        valn = list()
        for val in vals:
            valn.append(get_val(val))                   # 数値に変換して追加
            # Noneは除去しない。Noneも代入
        dev_vals[dev].append(valn)                      # 配列valnを追加
        dev_date[dev].append(date)                      # 時刻dateを追加
        while len(dev_vals[dev]) > HIST_BUF_N:          # 履歴保持数を超過
            del dev_vals[dev][0]                        # 最も古いデータを削除
            del dev_date[dev][0]                        # 最も古いデータを削除
        # print(dev_vals)
sock.close()                                            # ソケットの切断
