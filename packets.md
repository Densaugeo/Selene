# Selene Packet Definitions

Selene packets are used by Selene devices to describe pin changes, general device and pin info, connection state, and to make requests for changes. A μC can run one or more Selene device, publish packets related to it, and listen for request packets.

An MQTT broker is used for dispatching Selene packets over networks. Selene packets are sent with QoS = 0, but with the retain flag set to true - this means packets are not guaranteed to be delivered, and the broker saves the most recent packet in each topic. Relay nodes link μCs to the MQTT broker, and web clients can connect to MQTT brokers directly.

To support use in μCs, web browsers, and transmission over MQTT, Selene packets have three standard representations: binary, MQTT, and JavaScript.

## Binary Representation

|Byte #|Name     |Type  |Description|
|:----:|:-------:|:----:|:----------|
|0     |Prefix   |0x53  |For distinguishing from other prototcols. Always equal to ascii 'S'.|
|1-4   |Address  |u32LE*||
|5     |Type     |1-5   |Type code. Selene packet types are described in the 'Packet Types' section.|
|6     |Pin      |u8**  |Pin #. For packets that do not refer to pin (e.g. devinfo), equal to zero.|
|7     |Flags    |      ||
|7.0***|IsRequest|bool  |True for packets requesting a change. False for packets reporting actual state.|
|7.1-7 |Reserved |0     |Reserved for future use. Currently always equal to zero.|
|8-9   |Reserved |0     |Reserved for future use. Currently always equal to zero.|
|10    |Len      |u8    |Specifies payload length in bytes for packets with variables-size payloads. Equal to zero otherwise.|
|11-End|Payload  |?     |Payload type varies depeneding on packet type. Details in the 'Packet Types' section.|

\* 32-bit unsigned integer with little endian byte order.
\*\* 8-bit unsigned integer.
\*\*\* .0 is the most significant bit, .7 is leeast significant.

Note: 'u8' means 8-bit unsigned integer, 'u32LE' means 

## MQTT Representation

MQTT packets are divided into a topic (String) and a message (binary). For Selene, MQTT topics follow the pattern `'Se/[Address]/[Type]/[Pin]/[IsRequest]'`, with MQTT messages being equal to Selene payloads.

|Topic Section|Name     |Description|
|
|0|Prefix|For distinguishing from non-Selene MQTT packets. Always equal to "Se".|
|1|Address|Hex string, in uppercase, with no leading zeroes. At most 8 characters, corresponding to 32-bit addresses|
|2|Type|Human-readable type name, in lowercase. Details in the 'Packet Types' section.|
|3|Pin|Hex string, in uppercase, with no leading zeroes. Omitted from packets that do not refer to pins (e.g. devinfo).
|End|IsRequest|Equal to "r" if packet is a request, otherwise omitted. May be in position 3 or 4, depending on whether topic has a pin value.|

Note: Trailing slashes and further subtopics are not permitted in Selene topics.

## JavaScript Representation

~~~
Packet {
  Number address,
  String type,
  Number|null pin,
  Buffer payload,
  Boolean isRequest
}
~~~

## Packet Types

|Code|Name      |Pin|Payload size|Payload type|Notes|
|:--:|:--------:|:-:|:----------:|:----------:|:----|
|1   |Discovery |No |0           |None        |When a μC receives a discovery packet, it should reply with devinfo and pininfo|
|2   |Connection|No |1           |bool        |Indicates if a μC is connected.
|3   |Devinfo   |No |Variable    |[u8]*       |Payload is typically a utf8-encoded JSON string.
|4   |Pininfo   |Yes|Variable    |[u8]*       |Payload is typically a utf8-encoded JSON string.
|5   |Pin       |Yes|4           |u32LE**     |Used to report pin values, or with the request flag to request a pin change.|

\* Array of 8-bit unsigned integers.
\*\* 32-bit unsigned integer with little endian byte order.

## Miscellaneous

Address 0xFFFFFFFF is all call. So, to discover all μCs on a bus, send a discovery packet with address 0xFFFFFFFF.

Type code 0 is not used. I had a reason for that, but I forget what it is.

## Examples

Connection packet indicating device 0xCA75 is connected:

~~~
// Binary
<Buffer 53 75 ca 00 00 02 00 00 00 00 00 01>

// MQTT
{
  topic: "Se/CA75/connection",
  message: <Buffer 01>
}

// JSON
{
  address: 0xCA75,
  type: "connection",
  pin: null,
  payload: <Buffer 01>,
  isRequest: false
}
~~~

Pininfo packet describing pin 2 on device 0xFAE:

~~~
// Binary
<Buffer 53 ae 0f 00 00 04 02 00 00 00 20 7b 22 6e 61 6d
        65 22 3a 22 4c 45 44 22 2c 22 6d 69 6e 22 3a 30
        2c 22 6d 61 78 22 3a 31 30 32 33>

// MQTT
{
  topic: 'Se/FAE/pininfo/2',
  message: <Buffer 7b 22 6e 61 6d 65 22 3a 22 4c 45 44 22 2c 22 6d
                   69 6e 22 3a 30 2c 22 6d 61 78 22 3a 31 30 32 33>
}

// JSON
{
  address: 0xFAE,
  type: "pininfo",
  pin: 2,
  payload: <Buffer 7b 22 6e 61 6d 65 22 3a 22 4c 45 44 22 2c 22 6d
                   69 6e 22 3a 30 2c 22 6d 61 78 22 3a 31 30 32 33>,
  isRequest: false
}
~~~

Pin packet requesting that device 0x5E34 set pin 0xA1 to 13:

~~~
// Binary
<Buffer 53 34 5e 00 00 05 a1 80 00 00 00 0d 00 00 00>

// MQTT
{
  topic: "Se/5E34/pin/A1/r",
  message: <Buffer 0d 00 00 00>
}

// JSON
{
  address: 0x5E34,
  type: "pin",
  pin: 0xA1,
  payload: <Buffer 0d 00 00 00>,
  isRequest: true
}
~~~
