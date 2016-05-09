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

var mqtt_to_server = mqtt.connect('mqtt://localhost:1883', {
  queueQoSZero: false,
  will: {
    topic: 'Se/1/connection',
    payload: Buffer([0]),
    retain: true
  }
});

logger.info('Connecting to mqtt://localhost:1883...');

var info = {
  name: 'Selene One',
  description: 'Very first Selene device'
}

var pin_0_info = {
  name: 'LED Select',
  description: 'Does exactly what it sounds like',
  min: 0,
  max: 2
}

mqtt_to_server.on('connect', function() {
  logger.info('Connected to mqtt://localhost:1883');
  
  mqtt_to_server.publish('Se/1/connection', Buffer([1]), { retain: true, qos: 0 });
  mqtt_to_server.publish('Se/1/devinfo', JSON.stringify(info), { retain: true, qos: 0  });
  mqtt_to_server.publish('Se/1/pin/0', Buffer([0]), { retain: true, qos: 0  });
  mqtt_to_server.publish('Se/1/pininfo/0', JSON.stringify(pin_0_info), { retain: true, qos: 0  });
});

mqtt_to_server.on('error', e => logger.error('MQTT client error:', e.toString()));

mqtt_to_server.on('message', function(topic, message) {
  logger.debug('MQTT received:', { topic: topic, message: message });
  
  var buffer = SeleneParsers.fromMQTTToBuffer(topic, message);
  
  if(buffer !== null) {
    skirnir.broadcast(buffer);
    
    logger.debug('Skirnir sent:', util.inspect(buffer));
  } else {
    logger.debug('Packet from MQTT was invalid');
  }
  
});

mqtt_to_server.subscribe('Se/1/pinreq/+');

///////////////////////
// Connection to μCs //
///////////////////////

var skirnir = new skirnir({dir: '/dev', autoscan: true, autoadd: true, baud: nconf.get('baud')});

// All packets received from Skirnir are sent through the WebSocket
skirnir.on('message', function(e) {
  logger.debug('Received from ' + e.device + ': ' + util.inspect(new Buffer(e.data)));
  
  // If we have a Selene message
  var mqtt_message = SeleneParsers.fromBufferToMQTT(new Buffer(e.data));
  
  if(mqtt_message !== null) {
    mqtt_to_server.publish(mqtt_message[0], mqtt_message[1], { retain: true, qos: 0 });
    
    logger.debug('Sent to MQTT:', { topic: mqtt_message[0], message: util.inspect(mqtt_message[1]) });
  }
});

skirnir.on('connect', e => {
  logger.info('Connected device ' + e.device);
  
  skirnir.connections[e.device].send(SeleneParsers.makeDiscoveryPacket(0xFFFFFFFF));
  
  logger.debug('Skirnir sent to ' + e.device + ':', util.inspect(buffer));
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
  cli.context.ws                 = ws;
  cli.context.persistent_ws      = persistent_ws;
  
//   cli.context.ws_to_server       = ws_to_server;
}
