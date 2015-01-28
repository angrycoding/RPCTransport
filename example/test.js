var SerialPort = require('serialport').SerialPort;


const WAIT_INTERVAL = 250;
const ARRAY_PUSH = Array.prototype.push;
const ARRAY_FOREACH = Array.prototype.forEach;

const RPC_NULL = 0;
const RPC_BOOL = 1;
const RPC_FLOAT = 2;
const RPC_INT = 3;
const RPC_STRING = 4;

const RPC_START = 5;
const RPC_ARGUMENTS = 6;
const RPC_ARGUMENT = 7;
const RPC_END = 8;

const RPC_CMD_CALL = 0x10;
const RPC_CMD_RET = 0x20;
const RPC_CMD_BIND = 0x30;
const RPC_CMD_READY = 0x40;


function waitConnection(serialPort) {
	serialPort.open(function(error) {
		if (error) setTimeout(
			waitConnection,
			WAIT_INTERVAL,
			serialPort
		);
	});
}

function createPacket() {
	var buffer = [0x7B, 0x7B, arguments.length];
	ARRAY_FOREACH.call(arguments, function(argument) {

		if (argument === undefined || argument === null) {
			buffer.push(RPC_NULL);
		}

		else if (typeof argument === 'boolean') {
			buffer.push(RPC_BOOL, Number(argument));
		}

		else if (typeof argument === 'number') {
			var temp = new Buffer(4);
			if (argument % 1) {
				buffer.push(RPC_FLOAT);
				temp.writeFloatLE(argument, 0);
			} else {
				buffer.push(RPC_INT);
				temp.writeInt32LE(argument, 0);
			}
			ARRAY_PUSH.apply(buffer, temp);
		}

		else if (typeof argument === 'string') {
			var temp = new Buffer(argument);
			buffer.push(RPC_STRING, temp.length);
			ARRAY_PUSH.apply(buffer, temp);
		}

	});
	buffer.push(0x7D, 0x7D);
	return buffer;
}


Transport.prototype.processPacket = function(data) {
	var argValues = this.argValues;

	if (argValues[0] === RPC_CMD_BIND) {
		this.bindings[argValues[1]] = argValues[2];
	}

	else if (argValues[0] === RPC_CMD_READY) {
		console.info('ready');
	}

	else {
		console.info('processPacket', this.argValues);
	}

};

function Transport(comName, baudRate) {

	var self = this;
	var serialPort = new SerialPort(comName, {baudRate: baudRate}, false);

	this.buffer = [];
	this.argCount = 0;
	this.argValues = [];
	this.state = RPC_START;
	this.bindings = {};

	serialPort.on('data', function(data) { self.processIncoming(data); });

	serialPort.on('open', function() {
		serialPort.emit('data', createPacket(RPC_CMD_READY));

		// serialPort.emit('data', createPacket(RPC_CMD_BIND, "changeState", 0));


	}.bind(this));

	serialPort.on('close', function() {
		// this.canSend = false;
	});

	waitConnection(serialPort);

}

Transport.prototype.processIncoming = function(data) {

	var available,
		gotPacket = false,
		state = this.state,
		buffer = this.buffer,
		argCount = this.argCount,
		argValues = this.argValues;

	ARRAY_PUSH.apply(buffer, data);

	loop: while (available = buffer.length) switch (state) {

		case RPC_START: {
			if (available < 2) break loop;
			if (buffer.shift() == 0x7B && buffer.shift() == 0x7B)
				state = RPC_ARGUMENTS;
			break;
		}

		case RPC_ARGUMENTS: {
			if (argValues.length = 0, argCount = buffer.shift())
				state = RPC_ARGUMENT;
			else state = RPC_END;
			break;
		}

		case RPC_ARGUMENT: {
			if (buffer[0] < RPC_START) {
				if (state = buffer.shift(), RPC_NULL == state) {
					argValues.push(null);
					state = (argValues.length < argCount ? RPC_ARGUMENT : RPC_END);
				}
			} else state = RPC_START;
			break;
		}

		case RPC_BOOL: {
			argValues.push(!!buffer.shift());
			state = (argValues.length < argCount ? RPC_ARGUMENT : RPC_END);
			break;
		}

		case RPC_FLOAT: {
			if (available < 4) break loop;
			argValues.push(new Buffer(buffer.splice(0, 4)).readFloatLE(0));
			state = (argValues.length < argCount ? RPC_ARGUMENT : RPC_END);
			break;
		}

		case RPC_INT: {
			if (available < 4) break loop;
			argValues.push(new Buffer(buffer.splice(0, 4)).readInt32LE(0));
			state = (argValues.length < argCount ? RPC_ARGUMENT : RPC_END);
			break;
		}

		case RPC_STRING: {
			if (available - 1 < buffer[0]) break loop;
			argValues.push(new Buffer(buffer.splice(0, buffer.shift())).toString());
			state = (argValues.length < argCount ? RPC_ARGUMENT : RPC_END);
			break;
		}

		case RPC_END: {
			if (available < 2) break loop;
			state = RPC_START;
			if (buffer[0] == 0x7D) buffer.shift(); else break;
			if (buffer[0] == 0x7D) buffer.shift(); else break;
			gotPacket = true;
			break loop;
		}


	}

	this.state = state;
	this.argCount = argCount;
	if (gotPacket) this.processPacket();

};



var transport = new Transport('/dev/cu.Bluetooth-Incoming-Port', 115200);

// transport.call('foo');

// /dev/cu.Bluetooth-Incoming-Port
// /dev/cu.Bluetooth-Modem

// waitPort('/dev/cu.Bluetooth-Modem', function(comName) {
// 	var serialPort = new SerialConnection(comName, {baudRate: 115200}, false);
// 	console.info(serialPort);
// });

// setInterval(function() {
// 	console.info(1111);
// }, 500);