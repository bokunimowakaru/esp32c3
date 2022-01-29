#!/usr/bin/env python3
# coding: utf-8

code     = ['aa','5a','8f','12','16','d1']   # リモコンコード(スペース区切り)

import raspi_ir
raspiIr = raspi_ir.RaspiIr('AEHA',out_port=4)

print('リモコン信号送信')
raspiIr.output(code)        # 赤外線リモコン信号を送信する
print(raspiIr.code)         # 送信した信号を表示する

'''
実行結果例
pi@raspberrypi4:~/iot/libs/ir_remote $ ./raspi_ir_out.py
リモコン信号送信
['aa', '5a', '8f', '12', '16', 'd1']

'''
