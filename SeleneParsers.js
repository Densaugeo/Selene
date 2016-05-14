/**
 * @description Parsers and converters for Selene events
 */

// Void was reserved
var unit = {
  coerce: () => null,
  pack: () => 0,
  unpack: () => null
}

var bool = {
  coerce: v => Boolean(v),
  pack: (buffer, v, offset) => buffer.writeUInt8(Boolean(v), offset),
  unpack: (buffer, offset) => Boolean(buffer.readUInt8(offset))
}

var u8 = {
  coerce: v => (v & 0xFF) >>> 0,
  pack: (buffer, v, offset) => buffer.writeUInt8(v, offset),
  unpack: (buffer, offset) => buffer.readUInt8(offset)
}

var u32LE = {
  coerce: v => (v & 0xFFFFFFFF) >>> 0,
  pack: (buffer, v, offset) => buffer.writeUInt32LE(v, offset),
  unpack: (buffer, offset) => buffer.readUInt32LE(offset)
}

var string_32 = {
  coerce: v => String(v),
  pack: (buffer, v, offset) => {
    var len = buffer.write(v, offset + 1, 32, 'utf8');
    return buffer.writeUInt8(len, offset);
  },
  unpack: (buffer, offset) => {
    var len = buffer.readUInt8(offset);
    return buffer.toString('utf8', offset + 1, offset + len + 1);
  }
}

var SeleneParsers = exports;

SeleneParsers.Pin = {
  hasPin: true,
  payloadType: u32LE
}

SeleneParsers.Pinreq = {
  hasPin: true,
  payloadType: u32LE
}

SeleneParsers.Pininfo = {
  hasPin: true,
  payloadType: string_32
}

SeleneParsers.Devinfo = {
  hasPin: false,
  payloadType: string_32
}

SeleneParsers.Connection = {
  hasPin: false,
  payloadType: bool
}

SeleneParsers.Discovery = {
  hasPin: false,
  payloadType: unit
}

// @prop Object TYPES -- Holds descriptions of Selene packet types. Keys are type names and type codes
SeleneParsers.TYPES = {
  pin: SeleneParsers.Pin,
  pinreq: SeleneParsers.Pinreq,
  pininfo: SeleneParsers.Pininfo,
  devinfo: SeleneParsers.Devinfo,
  connection: SeleneParsers.Connection,
  discovery: SeleneParsers.Discovery,
  
  0: SeleneParsers.Pin,
  1: SeleneParsers.Pinreq,
  2: SeleneParsers.Pininfo,
  3: SeleneParsers.Devinfo,
  4: SeleneParsers.Connection,
  5: SeleneParsers.Discovery
}

for(var i in SeleneParsers.TYPES) {
  if(isNaN(parseInt(i))) {
    SeleneParsers.TYPES[i].typeName = i;
  } else {
    SeleneParsers.TYPES[i].typeCode = i;
  }
}

// @method Buffer makeDiscoveryPacket(Number address) -- Make a Buffer-format discovery packet. Use 0xFFFFFFFF to call all addresses
SeleneParsers.makeDiscoveryPacket = function(address) {
  var buffer = new Buffer(45).fill(0);
  
  buffer.writeUInt8(83, 0); // Prefix - 'S'
  buffer.writeUInt32LE(address, 1); // Address
  buffer.writeUInt8(SeleneParsers.Discovery.typeCode, 5);
  
  return buffer;
}

// @method Number getAddressFromBuffer(Buffer buffer) -- Does what it says
SeleneParsers.getAddressFromBuffer = function(buffer) {
  return buffer.readUInt32LE(1);
}

// @method Number getTypeCodeFromBuffer(Buffer buffer) -- Does what it says
SeleneParsers.getTypeCodeFromBuffer = function(buffer) {
  return buffer.readUInt8(5);
}

// @method Buffer fromMQTTToBuffer(String topic, String message) -- Moves Selene packet from an MQTT topic+message to a Buffer
SeleneParsers.fromMQTTToBuffer = function(topic, message) {
  var topic_tree = topic.split('/');
  
  if(topic_tree[0] !== 'Se') {
    return null;
  }
  
  var type = SeleneParsers.TYPES[topic_tree[2]];
  
  if(type === undefined) {
    return null;
  }
  
  var buffer = new Buffer(45).fill(0);
  
  buffer.writeUInt8(83, 0); // Prefix - 'S'
  buffer.writeUInt32LE(topic_tree[1], 1); // Address
  buffer.writeUInt8(type.typeCode, 5);
  
  if(type.hasPin) {
    buffer.writeUInt8(topic_tree[3], 6);
  }
  
  type.payloadType.pack(buffer, message, 11);
  
  return buffer;
}

// @method Object fromMQTTToJS(String topic, String message) -- Moves Selene packet from an MQTT topic+message to a JS object
SeleneParsers.fromMQTTToJS = function(topic, message) {
  var topic_tree = topic.split('/');
  
  if(topic_tree[0] !== 'Se') {
    return null;
  }
  
  var type = SeleneParsers.TYPES[topic_tree[2]];
  
  if(type === undefined) {
    return null;
  }
  
  var event = {};
  
  event.address = u32LE.coerce(topic_tree[1]);
  event.typeCode = type.typeCode;
  event.type = type.typeName;
  
  if(type.hasPin) {
    event.pin = u8.coerce(topic_tree[3]);
  }
  
  event.value = type.payloadType.coerce(message);
  
  return event;
}

// @method [String, String] fromBufferToMQTT(Buffer buffer) -- Moves Selene packet from a Buffer to an MQTT topic+message
SeleneParsers.fromBufferToMQTT = function(buffer) {
  if(buffer[0] != 83) {
    return null;
  }
  
  var type = SeleneParsers.TYPES[buffer.readUInt8(5)];
  
  if(type === undefined) {
    return null;
  }
  
  var topic = 'Se/';
  
  topic += buffer.readUInt32LE(1) + '/'; // Address
  topic += type.typeName;
  
  if(type.hasPin) {
    topic += '/' + buffer.readUInt8(6);
  }
  
  var message = String(type.payloadType.unpack(buffer, 11));
  
  return [topic, message];
}

// @method [String, String] fromJSToMQTT(Object js) -- Moves Selene packet from a js object to an MQTT topic+message
SeleneParsers.fromJSToMQTT = function(js) {
  var type = SeleneParsers.TYPES[js.typeCode] || SeleneParsers.TYPES[js.type];
  
  if(type === undefined) {
    return null;
  }
  
  var topic = 'Se/';
  topic += u32LE.coerce(js.address) + '/'; // Address
  topic += type.typeName;
  
  if(type.hasPin) {
    topic += '/' + u8.coerce(js.pin);
  }
  
  var message = String(type.coerce(js.value));
  
  return [topic, message];
}
