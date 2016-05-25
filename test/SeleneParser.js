var SeleneParser = require('../SeleneParser.js');
var SelenePacket = SeleneParser.Packet;

var assert = require('assert');


var pininfo = { name: 'Foo', description: 'A wild description has appeared!', min: 0, max: 0x1FF };
var pininfo_string = JSON.stringify(pininfo);
var pininfo_buffer = new Buffer(pininfo_string);

var devinfo = { name: 'Foo', description: 'Look, a description!' };
var devinfo_string = JSON.stringify(devinfo);
var devinfo_buffer = new Buffer(devinfo_string);

var DISCOVERY = 1;
var CONNECTION = 2;
var DEVINFO = 3;
var PININFO = 4;
var PIN = 5;

var discovery_packet = {
  js: {
    address: 0xD15C0AEB,
    type: 'discovery',
    typeCode: DISCOVERY,
    typeSpec: SeleneParser.TYPES.discovery,
    pin: null,
    payload: new Buffer(0),
    value: null,
    isRequest: false
  },
  mqtt: {
    topic: 'Se/D15C0AEB/discovery',
    message: new Buffer(0)
  },
  buffer: new Buffer(['S'.charCodeAt(0), 0xEB, 0x0A, 0x5C, 0xD1, DISCOVERY, 0, 0, 0, 0, 0])
}

var connection_packet = {
  js: {
    address: 0x1,
    type: 'connection',
    typeCode: CONNECTION,
    typeSpec: SeleneParser.TYPES.connection,
    pin: null,
    payload: new Buffer([1]),
    value: true,
    isRequest: false
  },
  mqtt: {
    topic: 'Se/1/connection',
    message: new Buffer([1])
  },
  buffer: new Buffer(['S'.charCodeAt(0), 0x01, 0, 0, 0, CONNECTION, 0, 0, 0, 0, 0, 1])
}

var devinfo_packet = {
  js: {
    address: 0x12345678,
    type: 'devinfo',
    typeCode: DEVINFO,
    typeSpec: SeleneParser.TYPES.devinfo,
    pin: null,
    payload: devinfo_buffer,
    value: devinfo_string,
    isRequest: false
  },
  mqtt: {
    topic: 'Se/12345678/devinfo',
    message: devinfo_buffer
  },
  buffer: Buffer.concat([new Buffer(['S'.charCodeAt(0), 0x78, 0x56, 0x34, 0x12, DEVINFO, 0, 0, 0, 0, devinfo_buffer.length]), devinfo_buffer])
}

var devinfo_r_packet = {
  js: {
    address: 0xF00BA,
    type: 'devinfo',
    typeCode: DEVINFO,
    typeSpec: SeleneParser.TYPES.devinfo,
    pin: null,
    payload: devinfo_buffer,
    value: devinfo_string,
    isRequest: true
  },
  mqtt: {
    topic: 'Se/F00BA/devinfo/r',
    message: devinfo_buffer
  },
  buffer: Buffer.concat([new Buffer(['S'.charCodeAt(0), 0xBA, 0, 0x0F, 0, DEVINFO, 0, 0x80, 0, 0, devinfo_buffer.length]), devinfo_buffer])
}

var pininfo_packet = {
  js: {
    address: 0xFAE,
    type: 'pininfo',
    typeCode: PININFO,
    typeSpec: SeleneParser.TYPES.pininfo,
    pin: 0xAB,
    payload: pininfo_buffer,
    value: pininfo_string,
    isRequest: false
  },
  mqtt: {
    topic: 'Se/FAE/pininfo/AB',
    message: pininfo_buffer
  },
  buffer: Buffer.concat([new Buffer(['S'.charCodeAt(0), 0xAE, 0x0F, 0, 0, PININFO, 0xAB, 0, 0, 0, pininfo_buffer.length]), pininfo_buffer])
}

var pininfo_r_packet = {
  js: {
    address: 0x1024,
    type: 'pininfo',
    typeCode: PININFO,
    typeSpec: SeleneParser.TYPES.pininfo,
    pin: 0x0,
    payload: pininfo_buffer,
    value: pininfo_string,
    isRequest: true
  },
  mqtt: {
    topic: 'Se/1024/pininfo/0/r',
    message: pininfo_buffer
  },
  buffer: Buffer.concat([new Buffer(['S'.charCodeAt(0), 0x24, 0x10, 0, 0, PININFO, 0, 0x80, 0, 0, pininfo_buffer.length]), pininfo_buffer])
}

var pin_packet = {
  js: {
    address: 0xC30,
    type: 'pin',
    typeCode: PIN,
    typeSpec: SeleneParser.TYPES.pin,
    pin: 0xD2,
    payload: new Buffer([2, 1, 0, 0]),
    value: 0x102,
    isRequest: false
  },
  mqtt: {
    topic: 'Se/C30/pin/D2',
    message: new Buffer([2, 1, 0, 0])
  },
  buffer: new Buffer(['S'.charCodeAt(0), 0x30, 0x0C, 0, 0, PIN, 0xD2, 0, 0, 0, 0, 0x02, 0x01, 0, 0])
}

var pin_r_packet = {
  js: {
    address: 0xCDC,
    type: 'pin',
    typeCode: PIN,
    typeSpec: SeleneParser.TYPES.pin,
    pin: 0x12,
    payload: new Buffer([0xBB, 0, 0, 0]),
    value: 0xBB,
    isRequest: true
  },
  mqtt: {
    topic: 'Se/CDC/pin/12/r',
    message: new Buffer([0xBB, 0, 0, 0])
  },
  buffer: new Buffer(['S'.charCodeAt(0), 0xDC, 0x0C, 0, 0, PIN, 0x12, 0x80, 0, 0, 0, 0xBB, 0, 0, 0])
}

var packets = [discovery_packet, connection_packet, devinfo_packet, devinfo_r_packet, devinfo_empty_packet, pininfo_packet, pininfo_r_packet, pininfo_empty_packet, pin_packet, pin_r_packet];

var assert_packet_js = function(actual, expected, message) {
  var m = message || '';
  
  assert.ok(actual instanceof SelenePacket, m + '\u001b[90mBad packet - expected type \u001b[32mSelenePacket\u001b[90m, found type \u001b[31m' + actual.constructor.name + '\u001b[39m');
  assert.strictEqual(actual.address, expected.address, m + 'Bad .address');
  assert.strictEqual(actual.type, expected.type, m + 'Bad .type');
  assert.strictEqual(actual.typeCode, expected.typeCode, m + 'Bad .typeCode');
  assert.strictEqual(actual.typeSpec, expected.typeSpec, m + 'Bad .typeSpec');
  assert.strictEqual(actual.pin, expected.pin, m + 'Bad .pin');
  assert.deepStrictEqual(actual.payload, expected.payload, m + 'Bad .payload');
  assert.strictEqual(actual.value, expected.value, m + 'Bad .value');
  assert.strictEqual(actual.isRequest, expected.isRequest, m + 'Bad .isRequest');
}

suite('SelenePacket.fromMqtt()', function() {
  test('Parses valid discovery packet', function() {
    var packet = SelenePacket.fromMqtt(discovery_packet.mqtt.topic, discovery_packet.mqtt.message);
    assert_packet_js(packet, discovery_packet.js);
  });
  
  test('Parses valid connection packet', function() {
    var packet = SelenePacket.fromMqtt(connection_packet.mqtt.topic, connection_packet.mqtt.message);
    assert_packet_js(packet, connection_packet.js);
  });
  
  test('Parses valid devinfo packet', function() {
    var packet = SelenePacket.fromMqtt(devinfo_packet.mqtt.topic, devinfo_packet.mqtt.message);
    assert_packet_js(packet, devinfo_packet.js);
  });
  
  test('Parses valid devinfo/r packet', function() {
    var packet = SelenePacket.fromMqtt(devinfo_r_packet.mqtt.topic, devinfo_r_packet.mqtt.message);
    assert_packet_js(packet, devinfo_r_packet.js);
  });
  
  test('Parses valid pininfo packet', function() {
    var packet = SelenePacket.fromMqtt(pininfo_packet.mqtt.topic, pininfo_packet.mqtt.message);
    assert_packet_js(packet, pininfo_packet.js);
  });
  
  test('Parses valid pininfo/r packet', function() {
    var packet = SelenePacket.fromMqtt(pininfo_r_packet.mqtt.topic, pininfo_r_packet.mqtt.message, true);
    assert_packet_js(packet, pininfo_r_packet.js);
  });
  
  test('Parses valid pin packet', function() {
    var packet = SelenePacket.fromMqtt(pin_packet.mqtt.topic, pin_packet.mqtt.message);
    assert_packet_js(packet, pin_packet.js);
  });
  
  test('Parses valid pin/r packet', function() {
    var packet = SelenePacket.fromMqtt(pin_r_packet.mqtt.topic, pin_r_packet.mqtt.message);
    assert_packet_js(packet, pin_r_packet.js);
  });
  
  test('Packets with invalid prefixes are marked null', function() {
    var packet = SelenePacket.fromMqtt('Sf/C30/pin/D2', new Buffer([1, 0, 0, 0]));
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromMqtt('C30/pin/D2', new Buffer([1, 0, 0, 0]));
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromMqtt('/C30/pin/D2', new Buffer([1, 0, 0, 0]));
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromMqtt('See/C30/pin/D2', new Buffer([1, 0, 0, 0]));
    assert.strictEqual(packet, null);
  });
  
  test('Packets with invalid addresses are marked null', function() {
    var packet = SelenePacket.fromMqtt('Se/bad_address/pin/D2', new Buffer([1, 0, 0, 0]));
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromMqtt('Se/100000000/pin/D2', new Buffer([1, 0, 0, 0]));
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromMqtt('Se/-1/pin/D2', new Buffer([1, 0, 0, 0]));
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromMqtt('Se/c30/pin/D2', new Buffer([1, 0, 0, 0]));
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromMqtt('Se/0C30/pin/D2', new Buffer([1, 0, 0, 0]));
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromMqtt('Se//pin/D2', new Buffer([1, 0, 0, 0]));
    assert.strictEqual(packet, null);
  });
  
  test('Packets with invalid types are marked null', function() {
    var packet = SelenePacket.fromMqtt('Se/C30/type', new Buffer([1, 0, 0, 0]));
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromMqtt('Se/C30/', new Buffer([1, 0, 0, 0]));
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromMqtt('Se/C30', new Buffer([1, 0, 0, 0]));
    assert.strictEqual(packet, null);
  });
  
  test('Packets with invalid pin #s are marked null', function() {
    var packet = SelenePacket.fromMqtt('Se/C30/pin/bad_#', new Buffer([1, 0, 0, 0]));
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromMqtt('Se/C30/pin/100', new Buffer([1, 0, 0, 0]));
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromMqtt('Se/C30/pin/-1', new Buffer([1, 0, 0, 0]));
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromMqtt('Se/C30/pin/d2', new Buffer([1, 0, 0, 0]));
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromMqtt('Se/C30/pin/0D2', new Buffer([1, 0, 0, 0]));
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromMqtt('Se/C30/pin/', new Buffer([1, 0, 0, 0]));
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromMqtt('Se/C30/pin', new Buffer([1, 0, 0, 0]));
    assert.strictEqual(packet, null);
  });
  
  test('Packets with extra topics are marked null', function() {
    var packet = SelenePacket.fromMqtt('Se/C30/pin/D2/', new Buffer([1, 0, 0, 0]));
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromMqtt('Se/C30/pin/D2/r/', new Buffer([1, 0, 0, 0]));
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromMqtt('Se/C30/devinfo/', devinfo_packet.mqtt.message);
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromMqtt('Se/C30/devinfo/r/', devinfo_packet.mqtt.message);
    assert.strictEqual(packet, null);
  });
  
  test('Discovery packets with invalid payloads are marked null', function() {
    var packet = SelenePacket.fromMqtt('Se/C30/discovery', new Buffer([0]));
    assert.strictEqual(packet, null);
  });
  
  test('Connection packets with invalid payloads are marked null', function() {
    var packet = SelenePacket.fromMqtt('Se/C30/connection', new Buffer([1, 2]));
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromMqtt('Se/C30/connection/D2', new Buffer([2]));
    assert.strictEqual(packet, null);
  });
  
  test('Devinfo packets with invalid payloads are marked null', function() {
    var packet = SelenePacket.fromMqtt('Se/C30/devinfo/D2', new Buffer(145));
    assert.strictEqual(packet, null);
  });
  
  test('Pininfo packets with invalid payloads are marked null', function() {
    var packet = SelenePacket.fromMqtt('Se/C30/pininfo/D2', new Buffer(145));
    assert.strictEqual(packet, null);
  });
  
  test('Pin packets with invalid payloads are marked null', function() {
    var packet = SelenePacket.fromMqtt('Se/C30/pin/D2', new Buffer([1, 0, 0, 0, 0]));
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromMqtt('Se/C30/pin/D2', new Buffer([1, 0, 0]));
    assert.strictEqual(packet, null);
  });
  
  test('Pin/r packets with invalid payloads are marked null', function() {
    var packet = SelenePacket.fromMqtt('Se/C30/pin/D2/r', new Buffer([1, 0, 0, 0, 0]));
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromMqtt('Se/C30/pin/D2/r', new Buffer([1, 0, 0]));
    assert.strictEqual(packet, null);
  });
});

suite('SelenePacket.fromBuffer()', function() {
  test('Parses valid discovery packet', function() {
    var packet = SelenePacket.fromBuffer(discovery_packet.buffer);
    assert_packet_js(packet, discovery_packet.js);
  });
  
  test('Parses valid connection packet', function() {
    var packet = SelenePacket.fromBuffer(connection_packet.buffer);
    assert_packet_js(packet, connection_packet.js);
  });
  
  test('Parses valid devinfo packet', function() {
    var packet = SelenePacket.fromBuffer(devinfo_packet.buffer);
    assert_packet_js(packet, devinfo_packet.js);
  });
  
  test('Parses valid devinfo/r packet', function() {
    var packet = SelenePacket.fromBuffer(devinfo_r_packet.buffer);
    assert_packet_js(packet, devinfo_r_packet.js);
  });
  
  test('Parses valid pininfo packet', function() {
    var packet = SelenePacket.fromBuffer(pininfo_packet.buffer);
    assert_packet_js(packet, pininfo_packet.js);
  });
  
  test('Parses valid pininfo/r packet', function() {
    var packet = SelenePacket.fromBuffer(pininfo_r_packet.buffer);
    assert_packet_js(packet, pininfo_r_packet.js);
  });
  
  test('Parses valid pin packet', function() {
    var packet = SelenePacket.fromBuffer(pin_packet.buffer);
    assert_packet_js(packet, pin_packet.js);
  });
  
  test('Parses valid pin/r packet', function() {
    var packet = SelenePacket.fromBuffer(pin_r_packet.buffer);
    assert_packet_js(packet, pin_r_packet.js);
  });
  
  test('Packets with invalid prefixes are marked null', function() {
    var packet = SelenePacket.fromBuffer(new Buffer([82, 0x30, 0x0C, 0, 0, PIN, 0xD2, 0, 0, 0, 0, 1, 0, 0, 0]));
    assert.strictEqual(packet, null);
  });
  
  test('Packets with invalid types are marked null', function() {
    var packet = SelenePacket.fromBuffer(new Buffer([83, 0x30, 0x0C, 0, 0, 0x06, 0xD2, 0, 0, 0, 0, 1, 0, 0, 0]));
    assert.strictEqual(packet, null);
  });
  
  test('Packets with non-zero reserved flags are marked null', function() {
    var packet = SelenePacket.fromBuffer(new Buffer([83, 0x30, 0x0C, 0, 0, PIN, 0xD2, 0x04, 0, 0, 0, 1, 0, 0, 0]), true);
    assert.strictEqual(packet, null);
  });
  
  test('Packets with non-zero reserved bytes are marked null', function() {
    var packet = SelenePacket.fromBuffer(new Buffer([83, 0x30, 0x0C, 0, 0, PIN, 0xD2, 0, 0x80, 0, 0, 1, 0, 0, 0]));
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromBuffer(new Buffer([83, 0x30, 0x0C, 0, 0, PIN, 0xD2, 0, 0, 0x01, 0, 1, 0, 0, 0]));
    assert.strictEqual(packet, null);
  });
  
  test('Packets with fixed-size payloads should not use len', function() {
    var packet = SelenePacket.fromBuffer(new Buffer([83, 0x30, 0x0C, 0, 0, PIN, 0xD2, 0, 0, 0, 0x01, 1, 0, 0, 0]));
    assert.strictEqual(packet, null);
  });
  
  test('Packets with not enough payload bytes are marked null', function() {
    var packet = SelenePacket.fromBuffer(new Buffer([83, 0x30, 0x0C, 0, 0, CONNECTION, 0xD2, 0, 0, 0, 0]));
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromBuffer(new Buffer([83, 0x30, 0x0C, 0, 0, PIN, 0xD2, 0, 0, 0, 0, 1, 0, 0]));
    assert.strictEqual(packet, null);
  });
  
  test('Packets with variable length payloads that are too short are marked null', function() {
    var packet = SelenePacket.fromBuffer(new Buffer([83, 0x30, 0x0C, 0, 0, DEVINFO, 0xD2, 0, 0, 0, 0x05, 1, 0, 0, 0]));
    assert.strictEqual(packet, null);
    
    packet = SelenePacket.fromBuffer(new Buffer([83, 0x30, 0x0C, 0, 0, PININFO, 0xD2, 0, 0, 0, 0x05, 1, 0, 0, 0]));
    assert.strictEqual(packet, null);
  });
  
  test('Devinfo packets with payloads longer than 144 bytes are marked null', function() {
    var buffer = Buffer.concat([new Buffer([83, 0x30, 0x0C, 0, 0, DEVINFO, 0xD2, 0, 0, 0, 0x91]), new Buffer(145)]);
    
    var packet = SelenePacket.fromBuffer(buffer);
    assert.strictEqual(packet, null);
  });
  
  test('Pininfo packets with payloads longer than 144 bytes are marked null', function() {
    var buffer = Buffer.concat([new Buffer([83, 0x30, 0x0C, 0, 0, PININFO, 0xD2, 0, 0, 0, 0x91]), new Buffer(145)]);
    
    var packet = SelenePacket.fromBuffer(buffer);
    assert.strictEqual(packet, null);
  });
});

suite('SelenePacket.prototype.toMqtt()', function() {
  test('Parses valid discovery packet', function() {
    var js = discovery_packet.js;
    var mqtt = new SelenePacket(js.address, js.typeSpec, js.pin, js.payload, js.isRequest).toMqtt();
    assert.deepStrictEqual(mqtt, discovery_packet.mqtt);
  });
  
  test('Parses valid connection packet', function() {
    var js = connection_packet.js;
    var mqtt = new SelenePacket(js.address, js.typeSpec, js.pin, js.payload, js.isRequest).toMqtt();
    assert.deepStrictEqual(mqtt, connection_packet.mqtt);
  });
  
  test('Parses valid devinfo packet', function() {
    var js = devinfo_packet.js;
    var mqtt = new SelenePacket(js.address, js.typeSpec, js.pin, js.payload, js.isRequest).toMqtt();
    assert.deepStrictEqual(mqtt, devinfo_packet.mqtt);
  });
  
  test('Parses valid devinfo/r packet', function() {
    var js = devinfo_r_packet.js;
    var mqtt = new SelenePacket(js.address, js.typeSpec, js.pin, js.payload, js.isRequest).toMqtt();
    assert.deepStrictEqual(mqtt, devinfo_r_packet.mqtt);
  });
  
  test('Parses valid pininfo packet', function() {
    var js = pininfo_packet.js;
    var mqtt = new SelenePacket(js.address, js.typeSpec, js.pin, js.payload, js.isRequest).toMqtt();
    assert.deepStrictEqual(mqtt, pininfo_packet.mqtt);
  });
  
  test('Parses valid pininfo/r packet', function() {
    var js = pininfo_r_packet.js;
    var mqtt = new SelenePacket(js.address, js.typeSpec, js.pin, js.payload, js.isRequest).toMqtt();
    assert.deepStrictEqual(mqtt, pininfo_r_packet.mqtt);
  });
  
  test('Parses valid pin packet', function() {
    var js = pin_packet.js;
    var mqtt = new SelenePacket(js.address, js.typeSpec, js.pin, js.payload, js.isRequest).toMqtt();
    assert.deepStrictEqual(mqtt, pin_packet.mqtt);
  });
  
  test('Parses valid pin/r packet', function() {
    var js = pin_r_packet.js;
    var mqtt = new SelenePacket(js.address, js.typeSpec, js.pin, js.payload, js.isRequest).toMqtt();
    assert.deepStrictEqual(mqtt, pin_r_packet.mqtt);
  });
});

suite('SelenePacket.prototype.toBuffer()', function() {
  test('Parses valid discovery packet', function() {
    var js = discovery_packet.js;
    var buffer = new SelenePacket(js.address, js.typeSpec, js.pin, js.payload, js.isRequest).toBuffer();
    assert.deepStrictEqual(buffer, discovery_packet.buffer);
  });
  
  test('Parses valid connection packet', function() {
    var js = connection_packet.js;
    var buffer = new SelenePacket(js.address, js.typeSpec, js.pin, js.payload, js.isRequest).toBuffer();
    assert.deepStrictEqual(buffer, connection_packet.buffer);
  });
  
  test('Parses valid devinfo packet', function() {
    var js = devinfo_packet.js;
    var buffer = new SelenePacket(js.address, js.typeSpec, js.pin, js.payload, js.isRequest).toBuffer();
    assert.deepStrictEqual(buffer, devinfo_packet.buffer);
  });
  
  test('Parses valid devinfo/r packet', function() {
    var js = devinfo_r_packet.js;
    var buffer = new SelenePacket(js.address, js.typeSpec, js.pin, js.payload, js.isRequest).toBuffer();
    assert.deepStrictEqual(buffer, devinfo_r_packet.buffer);
  });
  
  test('Parses valid pininfo/r packet', function() {
    var js = pininfo_r_packet.js;
    var buffer = new SelenePacket(js.address, js.typeSpec, js.pin, js.payload, js.isRequest).toBuffer();
    assert.deepStrictEqual(buffer, pininfo_r_packet.buffer);
  });
  
  test('Parses valid pininfo packet', function() {
    var js = pininfo_packet.js;
    var buffer = new SelenePacket(js.address, js.typeSpec, js.pin, js.payload, js.isRequest).toBuffer();
    assert.deepStrictEqual(buffer, pininfo_packet.buffer);
  });
  
  test('Parses valid pin packet', function() {
    var js = pin_packet.js;
    var buffer = new SelenePacket(js.address, js.typeSpec, js.pin, js.payload, js.isRequest).toBuffer();
    assert.deepStrictEqual(buffer, pin_packet.buffer);
  });
  
  test('Parses valid pin/r packet', function() {
    var js = pin_r_packet.js;
    var buffer = new SelenePacket(js.address, js.typeSpec, js.pin, js.payload, js.isRequest).toBuffer();
    assert.deepStrictEqual(buffer, pin_r_packet.buffer);
  });
});

suite('SelenePacket()', function() {
  test('Can make packets from fundamental properties', function() {
    packets.forEach(function(v, i) {
      var packet = new SelenePacket(v.js.address, v.js.typeSpec, v.js.pin, v.js.payload, v.js.isRequest);
      assert_packet_js(packet, v.js, 'Packet #' + i + ' - ');
    });
  });
  
  test('Can make packets using typecodes', function() {
    packets.forEach(function(v, i) {
      var packet = new SelenePacket(v.js.address, v.js.typeCode, v.js.pin, v.js.payload, v.js.isRequest);
      assert_packet_js(packet, v.js, 'Packet #' + i + ' - ');
    });
  });
  
  test('Can make packets using typenames', function() {
    packets.forEach(function(v, i) {
      var packet = new SelenePacket(v.js.address, v.js.type, v.js.pin, v.js.payload, v.js.isRequest);
      assert_packet_js(packet, v.js, 'Packet #' + i + ' - ');
    });
  });
  
  test('Can make packets using values in place of payloads', function() {
    packets.forEach(function(v, i) {
      var packet = new SelenePacket(v.js.address, v.js.typeSpec, v.js.pin, v.js.value, v.js.isRequest);
      assert_packet_js(packet, v.js, 'Packet #' + i + ' - ');
    });
  });
  
  test('Can make discovery packets with only address and type', function() {
    var packet = new SelenePacket(discovery_packet.js.address, discovery_packet.js.type);
    assert_packet_js(packet, discovery_packet.js);
  });
});
