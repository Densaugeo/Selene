process.title = 'selene-relay';

var nconf = require('nconf');
var repl = require('repl');
var skirnir = require('skirnir');
var util = require('util');
var winston = require('winston');
var ws = require('ws');

global.WebSocket = ws; // Supply WebSocket global for client-side persistent-ws library
var persistent_ws = require('persistent-ws');

//////////////
// Settings //
//////////////

nconf.argv().env();

nconf.file(__dirname + '/config.json');

nconf.defaults({
  remoteURL: 'ws://localhost:8088/',
  baud: 115200,
  silent: false,
  logLevelConsole: 'info',
  logLevelFile: 'info',
  logFile: __dirname + 'relay.log',
  logFileSize: 100*1024,
  reple: false
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

var ws_to_server = new persistent_ws(nconf.get('remoteURL'), undefined, {verbose: true, maxRetryTime: 30000, logger: logger.info});

// ws_to_server.send('foo');

ws_to_server.addEventListener('open', function() {
  // Just a small test
  ws_to_server.send(Buffer([1,2,3,4,5]));
});

// ws library doesn't emit a close event when it errors while connecting
ws_to_server.onerror = function(e) {
  if(e.syscall === 'connect') {
    ws_to_server.socket.emit('close');
  }
}

// All packets received from WebSocket are sent through Skirnir
ws_to_server.onmessage = function(e) {
  logger.debug('Received from WebSocket: ' + util.inspect(e.data));
  
  skirnir.broadcast(new Buffer(e.data));
}

///////////////////////
// Connection to μCs //
///////////////////////

var skirnir = new skirnir({dir: '/dev', autoscan: true, autoadd: true, baud: nconf.get('baud')});

// All packets received from Skirnir are sent through the WebSocket
skirnir.on('message', function(e) {
  logger.debug('Received from' + e.device + ': ' + util.inspect(new Buffer(e.data)));
  
  ws_to_server.send(new Buffer(e.data));
});

// Rest of these are just for logging
skirnir.on('add', function(e) {
  logger.info('Added new serial device: ' + e.device);
});

skirnir.on('remove', function(e) {
  logger.info('Removed serial device: ' + e.device);
});

skirnir.on('connect', function(e) {
  logger.info('Connected device ' + e.device);
});

skirnir.on('disconnect', function(e) {
  logger.info('Disconnected device ' + e.device);
});

skirnir.on('error', function(e) {
  logger.error('Error event from ' + e.call + ': ' + e.error);
});

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
  
  cli.context.ws_to_server       = ws_to_server;
}
