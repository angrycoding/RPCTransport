var RPCTransport = require('../nodejs/RPCTransport.js');
var transport = new RPCTransport('/dev/cu.usbserial-A965DFR7', 115200);
var robot = require("robotjs");
var mouse = robot.getMousePos();


var io = require('socket.io')(9999);




io.on('connect', function(socket) {

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

	transport.on('vcnl', function(args, ret) {
		socket.emit('vcnl', args, ret);
	});

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