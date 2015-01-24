var RPCTransport = require('../nodejs/RPCTransport.js');

var transport = new RPCTransport('/dev/cu.usbserial-A965DFR7', 115200);

transport.open(function(args, ret) {
	console.info('arduino calls nodejs with', args);
	ret(true);
	// setTimeout(function() {

	// 	transport.send();

	// }, 3000);
});

