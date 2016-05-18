process.title = 'selene-srvr';

var http = require('http');
var mosca = require('mosca');
var nconf = require('nconf');
var node_static = require('node-static');
var repl = require('repl');
var util = require('util');
var winston = require('winston');

//////////////
// Settings //
//////////////

nconf.argv().env();

nconf.file(__dirname + '/server_config.json');

nconf.defaults({
  host: '0.0.0.0',
  port: 8080,
  uCHost: '0.0.0.0',
  uCPort: 8088,
  silent: false,
  logLevelConsole: 'info',
  logLevelFile: 'info',
  logFile: __dirname + '/server.log',
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

winston.Logger.prototype.getHighestLogLevel = function() {
  var result = 0;
  
  for(var i in logger.transports) {
    result = Math.max(result, logger.levels[logger.transports[i].level || logger.level]);
  }
  
  return result;
}

var highest_log_level = logger.getHighestLogLevel();

process.on('uncaughtException', function (e) {
  logger.error('Uncaught ' + e.stack);
  process.exit(1);
});

/////////////////
// HTTP Server //
/////////////////

var file_server = new node_static.Server(__dirname + '/http');

var http_server = http.createServer(function(request, response) {
  request.on('end', function() {
    file_server.serve(request, response);
    logger.verbose(request.method + ' ' + request.url + ' - ' + response.statusCode);
  }).resume();
});

http_server.listen(nconf.get('port'), nconf.get('host'), function() {
  logger.info('HTTP server listening at http://' + http_server.address().address + ':' +  http_server.address().port + '/');
});

/////////////////
// MQTT Server //
/////////////////

var mosca_settings = {
  interfaces: [
    { type: 'mqtt', port: 1883 },
    { type: 'http', port: 8883 }
  ],
  persistence: {
    factory: mosca.persistence.Memory
  }
}

var mosca_server = new mosca.Server(mosca_settings);
mosca_server.on('ready', function() {
  logger.info('MQTT server listening at mqtt://0.0.0.0:1883/ and ws://0.0.0.0:8883/');
});

mosca_server.on('clientConnected', function(client) {
  logger.info('MQTT client connected:', { id: client.id });
});

mosca_server.on('clientDisconnecting', function(client) {
  logger.info('MQTT client disconnecting:', { id: client.id });
});

mosca_server.on('clientDisconnected', function(client) {
  logger.info('MQTT client disconnected:', { id: client.id });
});

mosca_server.on('subscribed', function(topic, client) {
  logger.debug('Client subscribed to topic:', { client: client.id, topic: topic });
});

mosca_server.on('unsubscribed', function(topic, client) {
  logger.debug('Client unsubscribed from topic:', { client: client.id, topic: topic });
});

mosca_server.on('published', function(packet, client) {
  if(packet.topic.substring(0, 5) !== '$SYS/' || highest_log_level >= logger.levels.silly) {
    var payload = packet.payload.length > 4 ? packet.payload.toString('UTF-8') : util.inspect(packet.payload);
    
    logger.debug('Packet published:', { client: client && client.id, topic: packet.topic, payload: payload });
    logger.silly('Packet details:', { messageId: packet.messageId, qos: packet.qos, retain: packet.retain });
  }
});

//////////
// REPL //
//////////

if(nconf.get('repl')) {
  var cli = repl.start({});
  
  cli.context.nconf              = nconf;
  cli.context.repl               = repl;
  cli.context.util               = util;
  cli.context.winston            = winston;
  
  cli.context.http_server = http_server;
  cli.context.logger = logger;
}
