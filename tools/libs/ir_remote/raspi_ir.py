#!/usr/bin/env python3
# coding: utf-8

import sys
import os
import subprocess

class RaspiIr:
    TYPES = ['AEHA','NEC','SIRC']                       # 赤外線リモコン方式名の一覧表

    def __init__(self,type='AEHA',in_port=0,out_port=0): # コンストラクタ作成
        self.dir = os.path.dirname(__file__)
        self.in_port  = in_port                         # GPIO ポート番号
        self.out_port = out_port                        # GPIO ポート番号
        self.ir_auto = False
        if type == 'AUTO':
            self.ir_auto = True
        self.ir_type  = type                            # 赤外線方式 AEHA/NEC/SIRC
        self.ir_wait_sec = -1                           # 最大待ち受け時間(-1で∞)
        self.code     = ['aa','5a','8f','12','16','d1'] # リモコンコード

    def input(self):
        if self.in_port == 0:
            raise Exception('ERROR: in_port=0')
        try:
            type_i = self.ir_type.index(self.ir_type)   # タイプ名の参照番号
        except ValueError as e:                         # 例外処理発生時(アクセス拒否)
            print('ERROR:ir_types,',e)                  # エラー内容表示
            type_i = 255                                # 赤外線リモコン方式を自動に設定
        if self.ir_auto or type_i == 3:                 # AUTOのとき
            type_i = 255                                # 赤外線リモコン方式を自動に設定
        path = self.dir + '/raspi_ir_in'                # IR 受信ソフトモジュールのパス
        app = [path, str(self.in_port), str(type_i), str(self.ir_wait_sec)] # 起動設定を集約
        res = subprocess.run(app, stdout=subprocess.PIPE)   # サブプロセスとして起動
        data = res.stdout.decode().strip()                  # 結果をdataへ代入
        ret = res.returncode                                # 終了コードをcodeへ代入
        leng = round((len(data)+1)/3)                       # 受信コード長をlengへ
        if ret != 0 or leng < 3:                            # 受信長3未満やエラー時
            print('app =', app)                         # サブ起動する設定内容を表示
            print('ret=', ret)                          # 結果データを表示
            print('len=', leng * 8)                     # 結果データを表示
            print('data=', data)                        # 結果データを表示
            raise Exception('ERROR: raspi_ir_in, return code='+ret)
        code = data.lower().split(' ')
        if type(code) is not list:
            raise Exception('ERROR: raspi_ir_in, got no list obj')
        self.code = code
        return code

    def output(self):
        return self.output(self, None)

    def output(self, code):
        if self.out_port == 0:
            raise Exception('ERROR: out_port=0')
        if code is None:
            code = self.code
        else:
            if type(code) is not list:
                raise Exception('argument is not list')
            self.code = code
        path = self.dir + '/raspi_ir_out'               # IR 送信ソフトモジュールのパス
        try:
            type_i = self.TYPES.index(self.ir_type)     # タイプ名の参照番号
        except ValueError as e:                         # 例外処理発生時(アクセス拒否)
            print('ERROR:ir type,',e)                   # エラー内容表示
            raise Exception('ir type not meet to TYPES:' + str(self.ir_type))
        app = [path, str(self.out_port), str(type_i)]   # 起動設定を集約
        for s in self.code:
            app.append(s)
        res = subprocess.run(app,stdout=subprocess.PIPE)# サブプロセスとして起動
        ret = res.returncode                            # 終了コードをretへ代入
        if ret != 0:
            data = res.stdout.decode().strip()          # 結果をdataへ代入
            print('app =', app)                         # サブ起動する設定内容を表示
            print('ret =', code)                        # 結果データを表示
            print('data=', data)                        # 結果データを表示
            raise Exception('ERROR: raspi_ir_out')
        return ret

    def __del__(self):                                  # インスタンスの削除
        if self.in_port > 0:
            path = self.dir + '/raspi_ir_in'            # IR 受信ソフトモジュールのパス
            app = [path, str(self.in_port), '-1']       # ポートの開放
            res = subprocess.run(app,stdout=subprocess.PIPE)  # サブプロセスとして起動
            if res.returncode != 0:                     # 終了コードを確認
                print(res.stdout.decode().strip())      # 結果を表示
                print('WARN: Failed to Disable Port',self.in_port)
        if self.out_port > 0:
            path = self.dir + '/raspi_ir_out'           # IR 送信ソフトモジュールのパス
            app = [path, str(self.out_port), '-1']      # ポートの開放
            res = subprocess.run(app,stdout=subprocess.PIPE)  # サブプロセスとして起動
            if res.returncode != 0:                     # 終了コードを確認
                print(res.stdout.decode().strip())      # 結果を表示
                print('WARN: Failed to Disable Port',self.out_port)

def main():
    raspiIr = RaspiIr('AEHA',in_port=18,out_port=4)

    print('リモコン信号受信中')
    code = raspiIr.input()      # 赤外線リモコン信号を受信する
    print(code)                 # 受信結果を表示(配列変数)
    print(raspiIr.code)         # 受信結果を表示(クラス内の変数・同じものが表示される)

    print('リモコン信号送信')
    raspiIr.output(code)        # 赤外線リモコン信号を送信する
    print(raspiIr.code)         # 送信した信号を表示する

    print('完了')

if __name__ == "__main__":
    main()

'''
実行結果例
pi@raspberrypi:~/iot/libs/ir-remote $ ./raspi_ir.py
リモコン信号受信中
['AA', '5A', '8F', '12', '16', 'D1']
['AA', '5A', '8F', '12', '16', 'D1']
リモコン信号送信
['AA', '5A', '8F', '12', '16', 'D1']
完了

'''
