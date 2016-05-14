process.title = 'selene-relay';

var mqtt = require('mqtt');
var nconf = require('nconf');
var repl = require('repl');
var skirnir = require('skirnir');
var util = require('util');
var winston = require('winston');

// Testing
var SeleneParsers = require('./SeleneParsers.js');

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

var pin_0_info = {
  name: 'LED Select',
  description: 'Does exactly what it sounds like',
  min: 0,
  max: 2
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
    
    mqtt_to_server.publish('Se/' + address + '/connection', Buffer([1]), { retain: true, qos: 0 });
    
    logger.debug('Sent to MQTT:', { topic: 'Se/' + address + '/connection', message: Buffer([1]) });
    
    mqtt_to_server.subscribe('Se/' + address + '/pinreq/+');
    
    for(var i in cache) {
      mqtt_to_server.publish(cache[i][0], cache[i][1], { retain: true, qos: 0 });
      
      logger.debug('Sent to MQTT:', { topic: cache[i][0], message: cache[i][1] });
    }
  });
  
  mqtt_to_server.on('error', e => logger.error('MQTT client error:', e.toString()));
  
  mqtt_to_server.on('message', onmessage);
  
  return mqtt_to_server;
}

var onmessage = function(topic, message) {
  logger.debug('MQTT received:', { topic: topic, message: message });
  
  var buffer = SeleneParsers.fromMQTTToBuffer(topic, message);
  
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
  
  // If we have a Selene message
  var mqtt_message = SeleneParsers.fromBufferToMQTT(buffer);
  
  if(mqtt_message !== null) {
    if(mqtt_caches[e.device] === undefined) {
      mqtt_caches[e.device] = {};
    }
    
    var typeCode = SeleneParsers.getTypeCodeFromBuffer(buffer);
    mqtt_caches[e.device][typeCode] = mqtt_message;
    
    if(mqtts[e.device] === undefined) {
      var address = SeleneParsers.getAddressFromBuffer(buffer);
      mqtts[e.device] = start_mqtt(address, mqtt_caches[e.device]);
    } else {
      mqtts[e.device].publish(mqtt_message[0], mqtt_message[1], { retain: true, qos: 0 });
      
      logger.debug('Sent to MQTT:', { topic: mqtt_message[0], message: mqtt_message[1] });
    }
  }
});

skirnir.on('connect', e => {
  logger.info('Connected device ' + e.device);
  
  var discovery_packet = SeleneParsers.makeDiscoveryPacket(0xFFFFFFFF);
  skirnir.connections[e.device].send(discovery_packet);
  
  logger.debug('Skirnir sent to ' + e.device + ':', util.inspect(discovery_packet));
});

// Rest of these are just for logging
skirnir.on('add'       , e => logger.info('Added new serial device: ' + e.device));
skirnir.on('remove'    , e => logger.info('Removed serial device: '   + e.device));
skirnir.on('disconnect', e => logger.info('Disconnected device '      + e.device));
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
  cli.context.SeleneParsers      = SeleneParsers;
}
