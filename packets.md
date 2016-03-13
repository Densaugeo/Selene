# Selene Packet Definitions

## How Packets Are Handled

At μC:

|Type               |Response|
|:-----------------:|:-------|
|Connect\|Disconnect|Do nothing|
|Query              |Reply with info/name/state|
|Info\|Name\|State  |Set given property|

μCs should generate state packets whenever their state changes, even if not requested by another packet (such as by a hardware control).

At relay:

|Type               |From  |Response|
|:-----------------:|:----:|:-------|
|Any                |μC    |Echo to servers|
|Any                |Server|Echo to μCs|

When a relay detects a new μC heartbeat, it should query the μC for its address and send a Connect packet. When the heartbeat fails, it should send a Disconnect packet. When the relay connects to a server, it should send Connect packets for all currently connected μCs.

At server:

|Type               |From  |Response|
|:-----------------:|:----:|:-------|
|Connect            |Relay |Add new μC, echo to clients, send Queries back|
|Disconnect         |Relay |Remove μC, echo to clients|
|Connect\|Disconnect|Client|Do nothing|
|Query              |Relay |Reply with info/name/state from cache|
|Query              |Client|Reply with info/name/state from cache|
|Info\|Name\|State  |Relay |Update cache, echo to clients if changed|
|Info\|Name\|State  |Client|Echo to relay if different from cache|

When a client connects to a server, the server should send Connect packets for all currently connected μCs.

At client:

|Type               |Response|
|:-----------------:|:-------|
|Connect            |Add new μC|
|Disconnect         |Remove μC|
|Query              |Reply with info/name/state from cache|
|Info\|Name\|State  |Update cache|

Clients should create Info/Name/State packets to control the μC.

## Packet Types

~~~
Packet {
  type: TypeCode,
  address: u32,
  payload: Payload,
}

enum TypeCode {
  Connect = 0,
  Disconnect = 1,
  Query = 2,
  Info = 3,
  Name = 4,
  State = 5,
}

enum Payload {
  Connect,
  Disconnect,
  Query,
  Info,
  Name,
  State,
}

Payload::Connect {}

Payload::Disconnect {}

Payload::Query {
  pin: u32,
  queryType: TypeCode,
}

Payload::Info {
  pin: u32,
  min: u64,
  max: u64,
}

Payload::Name {
  pin: u32,
  nameBytes: u8, // Max 16
  name: utf-8, // No more than 16 bytes
}

Payload::State {
  pin: u32,
  actual: u64,
  requested: u64,
}
~~~

Reserved numbers:

Address for all call is 0xFFFFFFFF

Pin # for μC info/name is 0

Pin # for all call is 0xFFFFFFFF
