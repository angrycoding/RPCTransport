var RPCTransport = require('../nodejs/RPCTransport.js');
var transport = new RPCTransport('/dev/cu.usbserial-A965DFR7', 115200);


var io = require('socket.io')(9999);

io.on('connect', function(socket) {

	transport.on('dht11', function(args, ret) {
		socket.emit('dht11', args, ret);
	});

	transport.on('bmp180', function(args, ret) {
		socket.emit('bmp180', args, ret);
	});

	transport.on('changed', function(args, ret) {
		socket.emit('changed', args, ret);
	});

	socket.on('rpc:call', function(name, args, ret) {
		transport.call(name, args, ret);
	});

	socket.on('rpc:notify', function(name, args, ret) {
		transport.notify(name, args, ret);
	});

});