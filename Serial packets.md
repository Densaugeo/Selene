# Serial Packet Definitions

These packets are used for the data link layer connection between microcontroller and higher-level PC. Supplies single-hop transport and a heartbeat, but not error detection.

Data packets:

|Byte   |0  |1-60   |61  |Notes|
|:-----:|:-:|:-----:|:--:|:----|
|       |'#'|Base64*|'\n'|Carries 60 bytes of encoded data, which is 45 bytes of raw data|

*Base64 format is '+' for 62, '/' for 63, and '=' for padding. Other non-control characters are interpreted as padding.

Heartbeat packets:

|Byte|0  |1   |Notes|
|:--:|:-:|:--:|:----|
|HB-1|'<'|'\n'|Initiates hearbeat; sent from Arduino|
|HB-2|'-'|'\n'|Reply from PC to Arduino|
|HB-3|'>'|'\n'|Reply from Arduino to PC|
