var RPCTransport = require('../nodejs/RPCTransport.js');
var transport = new RPCTransport('/dev/cu.usbserial-A965DFR7', 115200);


var io = require('socket.io')(9999);

io.on('connect', function(socket) {

	socket.on('changeState', function(state) {
		transport.call(0, state);
	})

});

transport.open(function(args, ret) {
// 	console.info('arduino calls nodejs with', args);
// 	ret(true);
// 	// setTimeout(function() {

// 	// 	transport.send();

// 	// }, 3000);
});

