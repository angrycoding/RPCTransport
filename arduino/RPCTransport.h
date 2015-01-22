#ifndef RPCTransport_h
#define RPCTransport_h

#include "RPCValue.h"
#include "RPCRequest.h"
#include "RPCResponse.h"

class RPCTransport {

private:

	enum {
		STATE_START = 20,
		STATE_ARGUMENTS = 21,
		STATE_ARGUMENT_START = 22,
		STATE_ARGUMENT_END = 23,
		STATE_END = 24
	};

	byte state;
	byte argCount;
	Stream *stream;
	RPCRequest* arguments;
	typedef void (*Handler) (RPCRequest&, RPCResponse&);
	byte handlerIndex;
	Handler handlers[10];

	void handleRequest(RPCRequest &request, RPCResponse &response) {
		char handler = request.getInt(0, -1);
		if (handler >= 0 &&
			handlers[handler] != NULL) {
			handlers[handler](request, response);
			response.unshiftString("RET");
		}
		else response.pushString("NOT_SUPPORTED");
	}


public:

	RPCTransport(Stream &stream) {
		argCount = 0;
		handlerIndex = 0;
		state = STATE_START;
		arguments = NULL;
		this->stream = &stream;
	}

	~RPCTransport() {
		delete arguments;
	}

	void on(char* name, Handler handler) {
		handlers[handlerIndex] = handler;

		RPCResponse response;
		response.pushString("$ATTACH");
		response.pushString(name);
		response.pushInt(handlerIndex++);

		writeResponse(response);
	}

	void begin() {

		RPCResponse response;
		response.pushString("$BEGIN");
		response.pushNull();
		response.pushBool(true);
		response.pushBool(false);
		response.pushFloat(3.14);
		response.pushInt(42);
		writeResponse(response);

	}


	void process() {
		byte size;
		while (size = stream->available()) switch (state) {

			case STATE_START: {
				if (size < 2) return;
				if (stream->read() == 0x7B && stream->read() == 0x7B)
					state = STATE_ARGUMENTS;
				break;
			}

			case STATE_ARGUMENTS: {
				if (arguments == NULL)
					arguments = new RPCRequest();
				else arguments->clear();
				if (argCount = stream->read()) {
					arguments->reserve(argCount);
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
				arguments->pushNull();
				state = STATE_ARGUMENT_END;
				break;
			}

			case RPCValue::Bool: {
				arguments->pushBool(!!stream->read());
				state = STATE_ARGUMENT_END;
				break;
			}

			case RPCValue::Float: {
				if (size < 4) return;
				char buffer[4];
				stream->readBytes(buffer, 4);
				arguments->pushFloat(*reinterpret_cast<float*>(&buffer));
				state = STATE_ARGUMENT_END;
				break;
			}

			case RPCValue::Int: {
				if (size < 4) return;
				char buffer[4];
				stream->readBytes(buffer, 4);
				arguments->pushInt(*reinterpret_cast<int32_t*>(&buffer));
				state = STATE_ARGUMENT_END;
				break;
			}

			case RPCValue::String: {
				if (size < stream->peek()) return;
				char value[(size = stream->read()) + 1];
				stream->readBytes(value, size);
				value[size] = '\0';
				arguments->pushString(value);
				state = STATE_ARGUMENT_END;
				break;
			}

			case STATE_ARGUMENT_END:
				if (arguments->length < argCount)
					state = STATE_ARGUMENT_START;
				else state = STATE_END;
				break;

			case STATE_END: {
				if (size < 2) return;
				state = STATE_START;
				if (stream->peek() == 0x7D) stream->read(); else break;
				if (stream->peek() == 0x7D) stream->read(); else break;

				RPCRequest* request = arguments;
				arguments = NULL;
				RPCResponse response;
				handleRequest(*request, response);
				writeResponse(response);
				delete request;


				// return true;
			}

		}
	}

	void writeResponse(RPCRequest &request) {
		stream->write(0x7B);
		stream->write(0x7B);
		request.write(stream);
		stream->write(0x7D);
		stream->write(0x7D);
	}

};

#endif