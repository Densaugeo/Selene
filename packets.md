# Selene Packet Definitions

## How Packets Are Handled

At μC:

|Type                 |Response|
|:-------------------:|:-------|
|Heartbeat            |Do nothing|
|Query                |Reply with info/name/state|
|Info \| Name \| State|Set given property|

μCs should generate state packets whenever their state changes, even if not requested by another packet (such as by a hardware control). μCs should generate heartbeats on a regular interval (maybe 5s?).

At relay:

|Type|From  |Response|
|:--:|:----:|:-------|
|Any |μC    |Echo to servers|
|Any |Server|Echo to μCs|

At server:

|Type                 |From  |Response|
|:-------------------:|:----:|:-------|
|Heartbeat            |Relay |Reset timeout, echo to clients; if new μC, update cache and send Queries|
|Heartbeat            |Client|Do nothing|
|Query                |Any   |Reply with info/name/state from cache|
|Info \| Name \| State|Relay |Update cache, echo to clients if changed|
|Info \| Name \| State|Client|Echo to relay if different from cache|

At client:

|Type                 |Response|
|:-------------------:|:-------|
|Heartbeat            |Reset timeout|
|Query                |Reply with info/name/state from cache|
|Info \| Name \| State|Update cache|

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
  Heartbeat,
  Query,
  Info,
  Name,
  State,
}

Payload::Heartbeat {}

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
