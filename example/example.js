var RPCTransport = require('../nodejs/RPCTransport.js');

//'/dev/cu.usbserial-A965DFR7'
// /dev/cu.Bluetooth-Modem

var io = require('socket.io')(9999);
var transport = RPCTransport('/dev/cu.Bluetooth-Incoming-Port', 115200);

transport.on('dht11', function(args, ret) {
	io.sockets.emit('dht11', args, ret);
});

transport.on('bmp180', function(args, ret) {
	io.sockets.emit('bmp180', args, ret);
});


/*

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
*/