/**
 * @description Parsers and converters for Selene packets
 * @depends Buffer
 */

// @prop Object TYPES -- Holds descriptions of Selene PacketTypes. Keys are type names and type codes
var TYPES = exports.TYPES = {}

/**
 * @module PacketType
 * @description Holds information on a Selene packet type
 */
var PacketType = exports.PacketType = function PacketType(options) {
  // @prop Boolean hasPin -- Does this type of packet have a pin #
  // @option Boolean hasPin -- Sets .hasPin
  this.hasPin = options.hasPin;
  
  // @prop Number size -- Size of payload in bytes. -1 if payload has variable size
  // @option Number size -- Sets .size
  this.size = options.size;
  
  // @method Boolean validate(Buffer payload) -- Tests if a payload is valid. Payloads are assumed to be buffers of the correct size
  // @option Function validate -- Sets .validate()
  this.validate = options.validate;
  
  // @method Buffer toBuffer(* v) -- Converts a value into a valid payload buffer
  // @option Function toBuffer -- Sets .toBuffer()
  this.toBuffer = options.toBuffer;
  
  // @method ? fromBuffer(Buffer buffer) -- Converts a valid payload buffer into a value
  // @option Function fromBuffer -- Sets .fromBuffer()
  this.fromBuffer = options.fromBuffer;
  
  // @prop Number typeCode -- Code # for type, used in binary representations
  // @prop String typeName -- Human-readable type name, used in MQTT topics
}

TYPES[1] = TYPES.discovery = new PacketType({
  hasPin: false,
  size: 0,
  validate: payload => true,
  toBuffer: (v) => new Buffer(0),
  fromBuffer: (buffer) => null
});

TYPES[2] = TYPES.connection = new PacketType({
  hasPin: false,
  size: 1,
  validate: payload => payload[0] <= 1,
  toBuffer: (v) => new Buffer([Boolean(v)]),
  fromBuffer: (buffer) => Boolean(buffer[0])
});

TYPES[3] = TYPES.devinfo = new PacketType({
  hasPin: false,
  size: -1,
  validate: payload => payload.length <= 144,
  toBuffer: (v) => new Buffer(v.substring(0, 144), 'utf8'),
  fromBuffer: (buffer) => buffer.toString('utf8')
});

TYPES[4] = TYPES.pininfo = new PacketType({
  hasPin: true,
  size: -1,
  validate: payload => payload.length <= 144,
  toBuffer: (v) => new Buffer(v.substring(0, 144), 'utf8'),
  fromBuffer: (buffer) => buffer.toString('utf8')
});

TYPES[5] = TYPES.pin = new PacketType({
  hasPin: true,
  size: 4,
  validate: payload => true,
  toBuffer: (v) => new Buffer([v, v >>> 8, v >>> 16, v >>> 24]),
  fromBuffer: (buffer) => buffer.readUInt32LE(0)
});

// Fill in .typeCode and .typeName on PacketTypes
for(var i in TYPES) {
  if(isNaN(parseInt(i))) {
    TYPES[i].typeName = i;
  } else {
    TYPES[i].typeCode = Number(i);
  }
}

/**
 * @module Packet
 * @description Represents a Selene packet
 * 
 * @example var Packet = require('./SelenePacket.js').Packet;
 * @example
 * @example // Read a packet from an MQTT message (saying pin 2 at device 0x3D is set to 5)
 * @example var packet = Packet.fromMqtt('Se/3D/pin/2', new Buffer([5, 0, 0, 0]));
 * @example packet.address; // 0x3D
 * @example packet.type; // 'pin'
 * @example packet.pin; // 2
 * @example packet.value; // 5
 * @example packet.isRequest; // false
 * @example
 * @example // Generate a packet and convert to an MQTT message (requesting device 0x3D to set pin 2 to 7)
 * @example var mqtt = new Packet(0x3D, 'pin', 2, 7, true).toMqtt();
 * @example mqtt.topic; 'Se/3D/pin/2/r'
 * @example mqtt.message; \<Buffer 07 00 00 00\>
 * @example
 * @example // Read a packet from a binary buffer (saying pin 2 at device 0xA0 is set to 1023)
 * @example var packet = Packet.fromBuffer(new Buffer([0x53, 0xA0, 0, 0, 0, 5, 2, 0, 0, 0, 0, 0xFF, 3, 0, 0]));
 * @example packet.address; // 0xA0
 * @example packet.type; // 'pin'
 * @example packet.pin; // 2
 * @example packet.value; // 1023
 * @example packet.isRequest; // false
 * @example
 * @example // Generate a packet and convert to a Buffer (requesting device 0xA0 to set pin 1 to 32)
 * @example var buffer = new Packet(0xA0, 'pin', 1, 32, true).toBuffer();
 * @example buffer; // \<Buffer 53 A0 00 00 00 05 01 80 00 00 00 20 00 00 00\>
 * @example
 * @example // Can chain buffer and MQTT conversions
 * @example var mqtt = Packet.fromBuffer(some_buffer).toMqtt();
 * @example var buffer = Packet.fromMqtt(some_topic, some_message).toBuffer();
 */
var Packet = exports.Packet = function Packet(address, type, pin, payload, isRequest) {
  // @prop Number address -- Selene address. Must be a 32-bit unsigned integer
  this.address = address;
  
  // @prop PacketType typeSpec -- Specifies packet type and related details. If set, will update .type and .typeCode
  this.typeSpec = type instanceof PacketType ? type : TYPES[type];
  
  // @prop Number pin -- Virtual pin # for a μC. Must be an 8-bit unsinged integer
  this.pin = pin === undefined ? null : pin;
  
  // @prop Buffer payload -- Binary payload sent in MQTT packets. If set, will update .value
  if(payload instanceof Buffer) {
    this.payload = payload;
  } else {
    this.value = payload;
  }
  
  // @prop Boolean isRequest -- Request flag
  this.isRequest = Boolean(isRequest);
}

Object.defineProperties(Packet.prototype, {
  // @prop String type -- Human-readable type name. If set, will update .typeCode and .typeSpec
  type: {
    get: function( ) { return this.typeSpec.typeName },
    set: function(v) { this.typeSpec = TYPES[v] }
  },
  // @prop Number typeCode -- Type code #. If set, will update .type and .typeSpec
  typeCode: {
    get: function( ) { return this.typeSpec.typeCode },
    set: function(v) { this.typeSpec = TYPES[v] }
  },
  // @prop ? value -- Value extracted from .payload. If set, will update .payload
  value: {
    get: function( ) { return this.typeSpec.fromBuffer(this.payload) },
    set: function(v) { this.payload = this.typeSpec.toBuffer(v) }
  }
});

// @method proto Boolean validate() -- Tests if a Packet is valid
Packet.prototype.validate = function() {
  return (
    this.address === (this.address & 0xFFFFFFFF) >>> 0 &&
    this.typeSpec instanceof PacketType &&
    this.pin === (this.typeSpec.hasPin ? (this.pin & 0xFF) >>> 0 : null) &&
    this.payload instanceof Buffer &&
    (this.typeSpec.size === -1 || this.payload.length === this.typeSpec.size) &&
    typeof this.isRequest === 'boolean' &&
    this.typeSpec.validate(this.payload)
  );
}

// @method Packet|null Packet.fromBuffer(Buffer buffer) -- Extracts Packets from valid buffers
Packet.fromBuffer = function(buffer) {
  var type = TYPES[buffer[5]];
  
  if(type === undefined) {
    return null;
  }
  
  var pSize = type.size !== -1 ? type.size : buffer[10];
  
  if(
    buffer[0] !== 83 || // Prefix - must be 'S'
    (!type.hasPin && buffer[6]) || // For packet types that do not refer to pins, pin byte must be zero
    buffer[7] & 0x7F || // Reserved flags must be zero
    buffer[8] & 0xFF || // Reserved bytes must be zero
    buffer[9] & 0xFF || // Reserved bytes must be zero
    (type.size !== -1 && buffer[10] !== type.size) || // For fixed-size packet types, pSize byte can be validated
    buffer.length < 11 + pSize || // Buffer must be long enough to hold payload
    pSize > 144 // Payloads > 144 bytes not yet supported
  ) {
    return null;
  }
  
  return new Packet(
    buffer.readUInt32LE(1),         // address
    type,                           // type
    type.hasPin ? buffer[6] : null, // pin
    buffer.slice(11, 11 + pSize),   // payload
    buffer[7] & 0x80                // isRequest
  );
}

// @method proto Buffer toBuffer() -- Does exactly what it says
Packet.prototype.toBuffer = function() {
  var buffer = new Buffer(11);
  
  buffer[0] = 83; // Prefix - 'S'
  buffer.writeUInt32LE(this.address, 1);
  buffer[5] = this.typeSpec.typeCode;
  buffer[6] = this.pin || 0;
  buffer[7] = this.isRequest << 7;
  buffer[8] = 0;
  buffer[9] = 0;
  buffer[10] = this.payload.length;
  
  return Buffer.concat([buffer, this.payload]);
}

// @method Packet|null Packet.fromMqtt(String topic, Buffer message) -- Extracts Packets from valid MQTT messages
Packet.fromMqtt = function(topic, message) {
  var topic_tree = topic.split('/');
  
  var typeSpec = TYPES[topic_tree[2]];
  
  if(typeSpec === undefined) {
    return null;
  }
  
  var address = parseInt(topic_tree[1], 16);
  var pin = typeSpec.hasPin ? parseInt(topic_tree[3], 16) : null;
  var isRequest = topic_tree[3 + typeSpec.hasPin] === 'r';
  
  if(
    topic_tree[0] !== 'Se' ||
    topic_tree[1] !== address.toString(16).toUpperCase() ||
    (typeSpec.hasPin && topic_tree[3] !== pin.toString(16).toUpperCase()) ||
    topic_tree[3 + typeSpec.hasPin + isRequest] !== undefined
  ) {
    return null;
  }
  
  var packet = new Packet(address, typeSpec, pin, message, isRequest);
  return packet.validate() ? packet : null;
}

// @method proto Object toMqtt() -- Converts Packet to { String topic, Buffer message } object
Packet.prototype.toMqtt = function() {
  var topic = 'Se/';
  
  topic += this.address.toString(16).toUpperCase();
  
  topic += '/' + this.type;
  
  if(this.typeSpec.hasPin) {
    topic += '/' + this.pin.toString(16).toUpperCase();
  }
  
  if(this.isRequest) {
    topic += '/r';
  }
  
  return { topic: topic, message: this.payload };
}
