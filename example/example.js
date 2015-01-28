var RPCTransport = require('../nodejs/RPCTransport.js');
// /dev/cu.usbserial-A965DFR7
var transport = new RPCTransport('/dev/cu.Bluetooth-Incoming-Port', 115200);


var io = require('socket.io')(9999);

transport.on('someNodeJSMethod', function(args, ret) {
	console.info('someNodeJSMethod', args);
	ret(1, 2, 3);
});

io.on('connect', function(socket) {

	// socket.on('changeState', function(state) {
	// 	transport.call(0, state);
	// });

});