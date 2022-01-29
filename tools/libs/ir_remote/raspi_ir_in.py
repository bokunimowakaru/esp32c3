#!/usr/bin/env python3
# coding: utf-8

import raspi_ir
raspiIr = raspi_ir.RaspiIr('AEHA',in_port=18)

print('リモコン信号受信中')
code = raspiIr.input()      # 赤外線リモコン信号を受信する
print(code)                 # 受信結果を表示

'''
実行結果例
pi@raspberrypi:~/iot/libs/ir_remote $ ./raspi_ir_in.py
リモコン信号受信中
['AA', '5A', '8F', '12', '16', 'D1']

'''
