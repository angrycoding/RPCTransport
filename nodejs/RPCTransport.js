var SerialPort = require('serialport').SerialPort;

const TYPE_NULL = 0;
const TYPE_BOOL = 1;
const TYPE_FLOAT = 2;
const TYPE_INT32 = 3;
const TYPE_STRING = 4;

const STATE_START = 10;
const STATE_ARGUMENTS = 11;
const STATE_ARGUMENT_START = 12;
const STATE_ARGUMENT_END = 13;
const STATE_END = 14;


function RPCTransport(port, baudrate) {

	var that = this;
	this.state = STATE_START;
	this.argValues = [];
	this.argCount = 0;
	var buffer = this.buffer = [];
	var serialPort = new SerialPort(port, {baudrate: baudrate}, false);

	serialPort.on('data', function(data) {
		// console.info(JSON.stringify(data));
		// console.info(data.toString());
		Array.prototype.push.apply(buffer, data);
		that.parsePacket();
	});

	this.serialPort = serialPort;
}

RPCTransport.prototype.parsePacket = function() {

	var that = this;
	var buffer = this.buffer;
	var state = this.state;
	var argCount = this.argCount;
	var argValues = this.argValues;

	var available;

	loop: while (available = buffer.length) switch (state) {

		case STATE_START: {
			if (available < 2) break loop;
			if (buffer.shift() == 0x7B && buffer.shift() == 0x7B)
				state = STATE_ARGUMENTS;
			break;
		}

		case STATE_ARGUMENTS: {
			if (argValues.length = 0, argCount = buffer.shift())
				state = STATE_ARGUMENT_START;
			else state = STATE_END;
			break;
		}

		case STATE_ARGUMENT_START: {
			if (buffer[0] < STATE_START)
				state = buffer.shift();
			else state = STATE_START;
			break;
		}

		case TYPE_NULL: {
			argValues.push(null);
			state = STATE_ARGUMENT_END;
			break;
		}

		case TYPE_BOOL: {
			argValues.push(!!buffer.shift());
			state = STATE_ARGUMENT_END;
			break;
		}

		case TYPE_FLOAT: {
			if (available < 4) break loop;
			argValues.push(new Buffer(buffer.splice(0, 4)).readFloatLE(0));
			state = STATE_ARGUMENT_END;
			break;
		}

		case TYPE_INT32: {
			if (available < 4) break loop;
			argValues.push(new Buffer(buffer.splice(0, 4)).readInt32LE(0));
			state = STATE_ARGUMENT_END;
			break;
		}

		case TYPE_STRING: {
			if (available < 1 || available - 1 < buffer[0]) break loop;
			argValues.push(new Buffer(buffer.splice(0, buffer.shift())).toString());
			state = STATE_ARGUMENT_END;
			break;
		}

		case STATE_ARGUMENT_END: {
			if (argValues.length < argCount)
				state = STATE_ARGUMENT_START;
			else state = STATE_END;
			break;
		}

		case STATE_END: {
			if (available < 2) break loop;
			state = STATE_START;
			if (buffer[0] == 0x7D) buffer.shift(); else break;
			if (buffer[0] == 0x7D) buffer.shift(); else break;

			console.info('arduino says', argValues);

			// if (argValues[0] === '$CALL') {
			// 	this.handleCall.call(this, argValues.slice(1), function(boolArgs) {

					var xBuff = [];
					Array.prototype.push.apply(xBuff, new Buffer('{{'));
					Array.prototype.push.apply(xBuff, [1]);

					Array.prototype.push.apply(xBuff, [TYPE_INT32, 0x20, 0x00, 0x00, 0x00]);

					// 52 69 84 0 // ret


					// Array.prototype.push.apply(xBuff, new Buffer('$RET'));

					// Array.prototype.push.apply(xBuff, [TYPE_STRING, '$RET'.length]);
					// Array.prototype.push.apply(xBuff, new Buffer('$RET'));
					// Array.prototype.push.apply(xBuff, [TYPE_BOOL, !!boolArgs]);
					Array.prototype.push.apply(xBuff, new Buffer('}}'));
					that.serialPort.write(xBuff, function() {});

			// 	});
			// }


			// // Array.prototype.push.apply(xBuff, [TYPE_STRING, '$CALL'.length]);
			// // Array.prototype.push.apply(xBuff, new Buffer('$CALL'));

			// Array.prototype.push.apply(xBuff, [TYPE_STRING, '$RET'.length]);
			// Array.prototype.push.apply(xBuff, new Buffer('$RET'));

			// // Array.prototype.push.apply(xBuff, [TYPE_BOOL, 0]);
			// Array.prototype.push.apply(xBuff, new Buffer('}}'));
			// this.serialPort.write(xBuff);

			// if (argValues[0] === '$ATTACH') {
			// 	attached[argValues[1]] = argValues[2];
			// 	continue;
			// }

			// else if (argValues[0] === '$BEGIN') {
			// 	doCall();
			// }

			// else {
			// 	doCall();
			// }

			// console.clear();
			// setTimeout(xfoo, 100);
			// xfoo();
			// break loop;
			break;
		}

		default: throw 'x';

	}

	this.state = state;
	this.argCount = argCount;

};

RPCTransport.prototype.open = function(handleCall) {
	this.handleCall = handleCall;
	this.serialPort.open(function(error) {
		if (error) console.info('error', error);
		console.info('connected');
	});
};

RPCTransport.prototype.send = function() {
	console.info('RPCTransport.prototype.send');
	var xBuff = [];
	Array.prototype.push.apply(xBuff, new Buffer('{{'));
	Array.prototype.push.apply(xBuff, [2]);

	Array.prototype.push.apply(xBuff, [TYPE_STRING, '$CALL'.length]);
	Array.prototype.push.apply(xBuff, new Buffer('$CALL'));

	Array.prototype.push.apply(xBuff, [TYPE_BOOL, 1]);

	Array.prototype.push.apply(xBuff, new Buffer('}}'));
	this.serialPort.write(xBuff);
};

RPCTransport.prototype.call = function(id, arg) {
	var xBuff = [];
	Array.prototype.push.apply(xBuff, new Buffer('{{'));
	Array.prototype.push.apply(xBuff, [3]);
	Array.prototype.push.apply(xBuff, [TYPE_INT32, 0x10, 0x00, 0x00, 0x00]);
	Array.prototype.push.apply(xBuff, [TYPE_INT32, id, 0x00, 0x00, 0x00]);
	Array.prototype.push.apply(xBuff, [TYPE_BOOL, Number(!!arg)]);
	Array.prototype.push.apply(xBuff, new Buffer('}}'));
	console.info('RPCTransport.prototype.call', xBuff);
	this.serialPort.write(xBuff);
};

module.exports = RPCTransport;