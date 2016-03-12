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

When a relay detects a new μC heartbeat, it should query the μC for its address and send a connect packet. When the heartbeat fails, it should send a disconnect packet.

At server:

|Type               |From  |Response|
|:-----------------:|:----:|:-------|
|Connect            |Relay |Add new μC, notify clients, send Queries back|
|Disconnect         |Relay |Remove μC, notify clients|
|Connect\|Disconnect|Client|Do nothing|
|Query              |Relay |Reply with info/name/state from requested cache|
|Query              |Client|Reply with info/name/state from actual cache|
|Info\|Name\|State  |Relay |Update actual cache, echo to clients|
|Info\|Name\|State  |Client|Update requested cache, echo to relay|

At client:

|Type               |Response|
|:-----------------:|:-------|
|Connect            |Add new μC|
|Disconnect         |Remove μC|
|Query              |Reply with info/name/state from requested cache|
|Info\|Name\|State  |Update actual cache|

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
  index: u32,
  queryType: TypeCode,
}

Payload::Info {
  index: u32,
  min: u64,
  max: u64,
}

Payload::Name {
  index: u32,
  nameBytes: u8, // Max 35
  name: utf-8, // No more than 35 bytes
}

Payload::State {
  index: u32,
  value: u64,
}
~~~

## Glossary

Requested cache: stores the state a user has requested for an actuator.

Actual cache: stores the state of an actuator, as reported by its μC.
