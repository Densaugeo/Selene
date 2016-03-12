# Selene Packet Definitions

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
