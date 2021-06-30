# Dhrystone.ino for ESP32

------------------------------------------------------------------------
## 本ソースコードの権利
# Dhrystone.ino
------------------------------------------------------------------------

2021/06.27に下記からダウンロードしたファイルを元に、ESP32-C3、ESP32、
ESP8266用に改変しました。  

https://github.com/ghalfacree/Arduino-Sketches/tree/master/Dhrystone

元ソースからの改変部の権利は、国野 亘 に帰属し、MITライセンスとします。  

Copyright (c) 2021 Wataru KUNINO
https://bokunimo.net/

Dhrystone.ino,
modified parts from followings are licensed under the MIT License.

------------------------------------------------------------------------
## 元ソースの権利情報  
# ghalfacree / Arduino-Sketches  
https://github.com/ghalfacree/Arduino-Sketches/tree/master/Dhrystone  
------------------------------------------------------------------------

 Dhrystone benchmark, modified for use with Arduinos and compatibles.
 Based on work by Ken Boak.
 Further modified to allow execution on an ATmega328-based board.
 Modified by Gareth Halfacree <gareth@halfacree.co.uk>

Gareth Halfacree
ghalfacree
Freelance Journalist, Technical Author
Bradford, UK https://freelance.halfacree.co.uk

------------------------------------------------------------------------
## 名称"DHRYSTONE" および 元ソースの参照元の権利  
# "DHRYSTONE" Benchmark Program  
------------------------------------------------------------------------
 
                    "DHRYSTONE" Benchmark Program
                    -----------------------------
 
   Version:    C, Version 2.1
 
   File:       dhry.h (part 1 of 3)
 
   Date:       May 25, 1988
 
   Author:     Reinhold P. Weicker
               Siemens Nixdorf Inf. Syst.
               STM OS 32
               Otto-Hahn-Ring 6
               W-8000 Muenchen 83
               Germany
                       Phone:    [+49]-89-636-42436
                                 (8-17 Central European Time)
                       UUCP:     weicker@ztivax.uucp@unido.uucp
                       Internet: weicker@ztivax.siemens.com
 
               Original Version (in Ada) published in
               "Communications of the ACM" vol. 27., no. 10 (Oct. 1984),
               pp. 1013 - 1030, together with the statistics
               on which the distribution of statements etc. is based.
 
               In this C version, the following C library functions are
               used:
               - strcpy, strcmp (inside the measurement loop)
               - printf, scanf (outside the measurement loop)
 
