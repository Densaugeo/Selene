process.title = 'selene-relay';

var mqtt = require('mqtt');
var nconf = require('nconf');
var repl = require('repl');
var skirnir = require('skirnir');
var util = require('util');
var winston = require('winston');

// Testing
var SeleneParser = require('./SelenePacket.js');

//////////////
// Settings //
//////////////

nconf.argv().env();

nconf.file(__dirname + '/relay_config.json');

nconf.defaults({
  remoteURL: 'ws://localhost:8088/',
  baud: 115200,
  silent: false,
  logLevelConsole: 'info',
  logLevelFile: 'info',
  logFile: __dirname + '/relay.log',
  logFileSize: 100*1024,
  repl: false
});

var logger = new winston.Logger({
  transports: [
    new winston.transports.Console({
      level: nconf.get('logLevelConsole'),
      silent: nconf.get('silent'),
      timestamp: true,
      colorize: true
    }),
    new winston.transports.File({
      level: nconf.get('logLevelFile'),
      silent: nconf.get('silent'),
      timestamp: true,
      filename: nconf.get('logFile'),
      json: false,
      maxFiles: 1,
      maxsize: nconf.get('logFileSize'),
      tailable: true
    }),
  ]
});

//////////////////////////
// Connection to server //
//////////////////////////

var mqtts = {};
var mqtt_caches = {};

mqtt.MqttClient.prototype.publishAndLog = function(topic, message, opts, callback) {
  this.publish(topic, message, opts, callback);
  
  logger.debug('Sent to MQTT:', { topic: topic, message: util.inspect(message) });
}

var start_mqtt = function(address, cache) {
  var mqtt_to_server = mqtt.connect('mqtt://localhost:1883', {
    queueQoSZero: false,
    will: {
      topic: 'Se/' + address + '/connection',
      payload: Buffer([0]),
      retain: true
    }
  });
  
  logger.info('Connecting to mqtt://localhost:1883...');
  
  mqtt_to_server.on('connect', function() {
    logger.info('Connected to mqtt://localhost:1883');
    
    mqtt_to_server.publishAndLog('Se/' + address + '/connection', Buffer([1]), { retain: true, qos: 0 });
    
    mqtt_to_server.subscribe('Se/' + address + '/pin/+/r');
    
    for(var i in cache) {
      mqtt_to_server.publishAndLog(cache[i].topic, cache[i].message, { retain: true, qos: 0 });
    }
  });
  
  mqtt_to_server.on('close', () => logger.info('Disconnected from mqtt://localhost:1883'));
  
  mqtt_to_server.on('error', e => logger.error('MQTT client error:', e.toString()));
  
  mqtt_to_server.on('message', onmessage);
  
  return mqtt_to_server;
}

var onmessage = function(topic, message) {
  logger.debug('MQTT received:', { topic: topic, message: message });
  
  var buffer = SeleneParser.Packet.fromMqtt(topic, message).toBuffer();
  
  if(buffer !== null) {
    skirnir.broadcast(buffer);
    logger.debug('Skirnir sent:', util.inspect(buffer));
  } else {
    logger.debug('Packet from MQTT was invalid');
  }
}

///////////////////////
// Connection to μCs //
///////////////////////

var skirnir = new skirnir({dir: '/dev', autoscan: true, autoadd: true, baud: nconf.get('baud')});

// All packets received from Skirnir are sent through the WebSocket
skirnir.on('message', function(e) {
  var buffer = new Buffer(e.data);
  
  logger.debug('Received from ' + e.device + ': ' + util.inspect(buffer));
  
  // If we have a Selene packet
  var packet = SeleneParser.Packet.fromBuffer(buffer);
  
  if(packet !== null) {
    var mqtt_message = packet.toMqtt();
    
    if(mqtt_caches[e.device] === undefined) {
      mqtt_caches[e.device] = {};
    }
    
    mqtt_caches[e.device][packet.typeCode] = mqtt_message;
    
    if(mqtts[e.device] === undefined) {
      mqtts[e.device] = start_mqtt(packet.address, mqtt_caches[e.device]);
    } else {
      mqtts[e.device].publishAndLog(mqtt_message.topic, mqtt_message.message, { retain: true, qos: 0 });
    }
  }
});

skirnir.on('connect', e => {
  logger.info('Connected device ' + e.device);
  
  var discovery_packet = new SeleneParser.Packet(0xFFFFFFFF, 'discovery').toBuffer();
  skirnir.connections[e.device].send(discovery_packet);
  
  logger.debug('Skirnir sent to ' + e.device + ':', util.inspect(discovery_packet));
});

skirnir.on('disconnect', e => {
  logger.info('Disconnected device ' + e.device);
  
  mqtts[e.device].publishAndLog('Se/1/connection', Buffer([0]), { retain: true, qos: 0 });
  mqtts[e.device].end(true, function() {
    mqtts[e.device].removeAllListeners();
    delete mqtts[e.device];
  });
});

// Rest of these are just for logging
skirnir.on('add'       , e => logger.info('Added new serial device: ' + e.device));
skirnir.on('remove'    , e => logger.info('Removed serial device: '   + e.device));
skirnir.on('error'     , e => logger.error('Error event from ' + e.call + ': ' + e.error));

//////////
// REPL //
//////////

if(nconf.get('repl')) {
  var cli = repl.start({});
  
  cli.context.nconf              = nconf;
  cli.context.repl               = repl;
  cli.context.skirnir            = skirnir;
  cli.context.util               = util;
  cli.context.winston            = winston;
  cli.context.SeleneParser       = SeleneParser;
}
