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
		Array.prototype.push.apply(buffer, data);
		that.parsePacket();
	});

	this.serialPort = serialPort;
}

RPCTransport.prototype.parsePacket = function() {

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
			if (available < 1 || available < buffer[0]) break loop;
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

RPCTransport.prototype.open = function() {
	this.serialPort.open(function(error) {
		if (error) console.info('error', error);
	});
};

module.exports = RPCTransport;