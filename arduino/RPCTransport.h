#ifndef RPCTransport_h
#define RPCTransport_h

#include "RPCValue.h"
#include "RPCPacket.h"

class RPCTransport {

friend class RPCPacket;

private:

	Stream *stream;
	RPCPacket* listener;
	typedef void (*Handler) (RPCPacket*);
	byte handlerIndex;
	Handler handlers[10];

public:

	RPCTransport(Stream &stream) {
		handlerIndex = 0;
		this->stream = &stream;
		listener = new RPCPacket(this);
	}

	void on(char* name, Handler handler) {
		handlers[handlerIndex] = handler;
	}

	void process() {
		listener->receive();
	}

	~RPCTransport() {
		delete listener;
	}


};




RPCPacket::RPCPacket(RPCTransport *transport): length(count) {
	state = STATE_START;
	this->transport = transport;
	this->stream = transport->stream;
	count = 0, reserved = 0, arguments = NULL;
}


RPCPacket::RPCPacket(RPCTransport &transport, RPCValue args[], byte argCount): length(count) {
	this->transport = &transport, stream = transport.stream;
	count = 0, reserved = 0, arguments = NULL, state = STATE_START;

	reserve(argCount);

	while (count < argCount) pushValue(args[count]);


	unshiftString("$CALL");
	send(true);
	// transport.handleResponse(this);
}


void RPCPacket::send(bool waitReturn) {

	byte c, vType;
	RPCValue* value;

	stream->write(0x7B);
	stream->write(0x7B);
	stream->write(count);

	for (c = 0; c < count; ++c) {
		value = arguments[c];

		switch (vType = value->vType) {

			case RPCValue::Null: {
				stream->write(vType);
				break;
			}

			case RPCValue::Bool: {
				byte buffer[2] = {vType, value->vBool ? 1 : 0};
				stream->write(buffer, 2);
				break;
			}

			case RPCValue::Float: {
				byte buffer[5] = {vType};
				*reinterpret_cast<float*>(&buffer[1]) = value->vFloat;
				stream->write(buffer, 5);
				break;
			}

			case RPCValue::Int: {
				byte buffer[5] = {vType};
				*reinterpret_cast<int32_t*>(&buffer[1]) = value->vInt;
				stream->write(buffer, 5);
				break;
			}

			case RPCValue::String: {
				byte length = strlen(value->vString);
				byte buffer[2 + length];
				buffer[0] = vType, buffer[1] = length;
				memcpy(&buffer[2], value->vString, length);
				stream->write(buffer, 2 + length);
				break;
			}

		}
	}

	stream->write(0x7D);
	stream->write(0x7D);
	stream->flush();

	clear();

	if (waitReturn) while (!receive());

	// stream->println("PROCESSED");

}


bool RPCPacket::receive() {
	byte size;

	while (size = stream->available()) switch (state) {

		case STATE_START: {
			if (size < 2) return false;
			if (stream->read() == 0x7B && stream->read() == 0x7B)
				state = STATE_ARGUMENTS;
			break;
		}

		case STATE_ARGUMENTS: {
			if (clear(), argCount = stream->read()) {
				reserve(argCount);
				state = STATE_ARGUMENT_START;
			} else state = STATE_END;
			break;
		}

		case STATE_ARGUMENT_START: {
			if (stream->peek() < STATE_START)
				state = stream->read();
			else state = STATE_START;
			break;
		}

		case RPCValue::Null: {
			pushNull();
			state = STATE_ARGUMENT_END;
			break;
		}

		case RPCValue::Bool: {
			pushBool(!!stream->read());
			state = STATE_ARGUMENT_END;
			break;
		}

		case RPCValue::Float: {
			if (size < 4) return false;
			char buffer[4];
			stream->readBytes(buffer, 4);
			pushFloat(*reinterpret_cast<float*>(&buffer));
			state = STATE_ARGUMENT_END;
			break;
		}

		case RPCValue::Int: {
			if (size < 4) return false;
			char buffer[4];
			stream->readBytes(buffer, 4);
			pushInt(*reinterpret_cast<int32_t*>(&buffer));
			state = STATE_ARGUMENT_END;
			break;
		}

		case RPCValue::String: {
			if (size - 1 < stream->peek()) return false;
			char value[(size = stream->read()) + 1];
			stream->readBytes(value, size);
			value[size] = '\0';
			pushString(value);
			state = STATE_ARGUMENT_END;
			break;
		}

		case STATE_ARGUMENT_END:
			if (count < argCount)
				state = STATE_ARGUMENT_START;
			else state = STATE_END;
			break;

		case STATE_END: {
			if (size < 2) return false;
			state = STATE_START;
			if (stream->peek() == 0x7D) stream->read(); else break;
			if (stream->peek() == 0x7D) stream->read(); else break;


			if (strcmp(getString(0), "$CALL") == 0) {
				transport->handlers[0](this);
				send(false);
				break;
			}

			else if (strcmp(getString(0), "$RET") == 0) {
				// stream->println("RECEIVED_RET");
				return true;
			}

			// if (call_received) {
			// 	handler = transport.getHandler();
			// 	handler(this);
			// 	// make_call_with_this_object
			// 	send(false);
			// }


			// RPCPacket* request = arguments;
			// arguments = NULL;
			// RPCResponse response;
			// handleRequest(*request, response);
			// writeResponse(response);
			// delete request;


			// return true;
		}

	}

	return false;

}





#define RPCPacket(transport, ...) RPCPacket(transport, (RPCValue[]){__VA_ARGS__}, strlen(#__VA_ARGS__) ? VA_NUM_ARGS(__VA_ARGS__) : 0)
#define VA_NUM_ARGS(...) VA_NUM_ARGS_IMPL(__VA_ARGS__, 5,4,3,2,1)
#define VA_NUM_ARGS_IMPL(_1,_2,_3,_4,_5,N,...) N

#endif