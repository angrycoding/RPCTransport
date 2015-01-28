var SerialPort = require('serialport').SerialPort;
var EventEmitter = require('events').EventEmitter;


const WAIT_INTERVAL = 250;

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

	var arg, temp, buffer = [0x7B, 0x7B, 0],
		args = Array.prototype.slice.call(arguments);

	while (args.length) {

		arg = args.shift();

		if (arg === undefined || arg === null) {
			buffer.push(RPC_NULL);
			buffer[2]++;
		}

		else if (typeof arg === 'boolean') {
			buffer.push(RPC_BOOL, Number(arg));
			buffer[2]++;
		}

		else if (typeof arg === 'number') {
			temp = new Buffer(4);
			if (arg % 1) {
				buffer.push(RPC_FLOAT);
				temp.writeFloatLE(arg, 0);
			} else {
				buffer.push(RPC_INT);
				temp.writeInt32LE(arg, 0);
			}
			Array.prototype.push.apply(buffer, temp);
			buffer[2]++;
		}

		else if (typeof arg === 'string') {
			temp = new Buffer(arg);
			buffer.push(RPC_STRING, temp.length);
			Array.prototype.push.apply(buffer, temp);
			buffer[2]++;
		}

		else if (arg instanceof Object) {
			arg = Array.prototype.slice.call(arg);
			Array.prototype.unshift.apply(args, arg);
		}

	}

	buffer.push(0x7D, 0x7D);
	return buffer;
}


function Transport(comName, baudRate) {

	var self = this;

	this.buffer = [];
	this.argCount = 0;
	this.argValues = [];
	this.state = RPC_START;
	this.bindings = {};

	var serialPort = new SerialPort(comName, {baudRate: baudRate}, false);

	serialPort.on('data', function(data) {
		self.processIncoming(data);
	});

	serialPort.on('open', function() {
		serialPort.write(createPacket(RPC_CMD_READY));
		serialPort.emit('data', createPacket(RPC_CMD_BIND, "changeState", 0));
		serialPort.emit('data', createPacket(RPC_CMD_BIND, "hello", 1));
		serialPort.emit('data', createPacket(RPC_CMD_READY));
		serialPort.emit('data', createPacket(RPC_CMD_CALL, "someNodeJSMethod", 1, 2, 3));
	});

	serialPort.on('close', function() {
		// this.canSend = false;
	});


	waitConnection(this.serialPort = serialPort);

}

Transport.prototype = Object.create(EventEmitter.prototype);


Transport.prototype.processPacket = function(data) {

	var argValues = this.argValues;
	var serialPort = this.serialPort;

	if (argValues[0] === RPC_CMD_BIND) {
		console.info('RPC_CMD_BIND', argValues);
		this.bindings[argValues[1]] = argValues[2];
	}

	else if (argValues[0] === RPC_CMD_READY) {
		console.info('RPC_CMD_READY', argValues);
	}

	else if (argValues[0] === RPC_CMD_CALL) {
		console.info('RPC_CMD_CALL', argValues);
		if (!this.emit(argValues[1], argValues.slice(1), function() {
			serialPort.write(createPacket(RPC_CMD_RET, arguments));
		})) serialPort.write(createPacket(RPC_CMD_RET));
	}

	else if (argValues[0] === RPC_CMD_RET) {
		console.info('RPC_CMD_RET', argValues);
	}

	else {
		console.info('processPacket', argValues);
	}

};

Transport.prototype.processIncoming = function(data) {

	var available,
		gotPacket = false,
		state = this.state,
		buffer = this.buffer,
		argCount = this.argCount,
		argValues = this.argValues;

	Array.prototype.push.apply(buffer, data);

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
			this.processPacket();
			break;
		}


	}

	this.state = state;
	this.argCount = argCount;

};


module.exports = Transport;