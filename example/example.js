var RPCTransport = require('../nodejs/RPCTransport.js');
var transport = RPCTransport('/dev/cu.usbserial-A965DFR7', 115200);
var robot = require("robotjs");
var mouse = robot.getMousePos();


var io = require('socket.io')(9999);


var lastCode = 0;
var timeout = 0;

transport.on('tsop', function(args, ret) {
	if (args[0] !== 4294967295) lastCode = args[0];
	clearTimeout(timeout);
	timeout = setTimeout(function() { lastCode = 0; }, 100);
});


setInterval(function() {

	if (lastCode === 1868153570) {
		robot.moveMouse(mouse.x--, mouse.y);
	}

	else if (lastCode === 3173119706) {
		robot.moveMouse(mouse.x++, mouse.y);
	}

	else if (lastCode === 872930847) {
		robot.moveMouse(mouse.x, mouse.y--);
	}

	else if (lastCode === 2354166757) {
		robot.moveMouse(mouse.x, mouse.y++);
	}

	else if (lastCode === 1481897729) {
		robot.mouseClick();
	}

}, 0);


transport.on('gas', function(args, ret) {
	io.sockets.emit('gas', args);
});

transport.on('bmp180', function(args, ret) {
	io.sockets.emit('bmp180', args);
});

transport.on('vcnl', function(args, ret) {
	io.sockets.emit('vcnl', args, ret);
});


transport.on('dht11', function(args, ret) {
	io.sockets.emit('dht11', args, ret);
});

io.on('connect', function(socket) {

	socket.on('toggleLed', function(ret) {
		transport.call('toggleLed', [], function() {
			ret.apply(this, arguments);
		});
	});

});