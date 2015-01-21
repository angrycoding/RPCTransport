#ifndef RPCTransport_h
#define RPCTransport_h

#include "RPCRequest.h"
#include "RPCResponse.h"

class RPCTransport {

private:

	enum State {
		STATE_NULL,
		STATE_BOOL,
		STATE_FLOAT,
		STATE_INT,
		STATE_STRING,
		STATE_START,
		STATE_ARGUMENTS,
		STATE_ARGUMENT_START,
		STATE_ARGUMENT_END,
		STATE_END
	};

	State state;
	byte argCount;
	Stream *stream;
	RPCRequest* arguments;
	typedef void (*Handler) (RPCRequest&, RPCResponse&);
	byte handlerIndex;
	Handler handlers[10];

	void handleRequest(RPCRequest &request, RPCResponse &response) {
		if (request.length &&
			request[0]->type == RPCValue::Int &&
			handlers[request[0]->vInt] != NULL) {
			handlers[request[0]->vInt](request, response);
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
				if (argCount = stream->read())
					state = STATE_ARGUMENT_START;
				else state = STATE_END;
				break;
			}

			case STATE_ARGUMENT_START: {
				switch (stream->peek()) {
					case RPCValue::Null: state = STATE_NULL; break;
					case RPCValue::Bool: state = STATE_BOOL; break;
					case RPCValue::Float: state = STATE_FLOAT; break;
					case RPCValue::Int: state = STATE_INT; break;
					case RPCValue::String: state = STATE_STRING; break;
					default: state = STATE_START; continue;
				}
				stream->read();
				break;
			}

			case STATE_NULL: {
				arguments->pushNull();
				state = STATE_ARGUMENT_END;
				break;
			}

			case STATE_BOOL: {
				arguments->pushBool(!!stream->read());
				state = STATE_ARGUMENT_END;
				break;
			}

			case STATE_FLOAT: {
				if (size < 4) return;
				char buffer[4];
				stream->readBytes(buffer, 4);
				arguments->pushFloat(*reinterpret_cast<float*>(&buffer));
				state = STATE_ARGUMENT_END;
				break;
			}

			case STATE_INT: {
				if (size < 4) return;
				char buffer[4];
				stream->readBytes(buffer, 4);
				arguments->pushInt(*reinterpret_cast<int32_t*>(&buffer));
				state = STATE_ARGUMENT_END;
				break;
			}

			case STATE_STRING: {
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
		stream->write(request.length);

		for (byte c = 0; c < request.length; c++) {
			RPCValue* value = request[c];
			switch (value->type) {

				case RPCValue::Null: {
					stream->write(value->_type);
					break;
				}

				case RPCValue::Bool: {
					byte buffer[2] = {value->_type, value->vBool ? 1 : 0};
					stream->write(buffer, 2);
					break;
				}

				case RPCValue::Float: {
					byte buffer[5] = {value->_type};
					*reinterpret_cast<float*>(&buffer[1]) = value->vFloat;
					stream->write(buffer, 5);
					break;
				}

				case RPCValue::Int: {
					byte buffer[5] = {value->_type};
					*reinterpret_cast<int32_t*>(&buffer[1]) = value->vInt;
					stream->write(buffer, 5);
					break;
				}

				case RPCValue::String: {
					byte length = strlen(value->vString);
					byte buffer[2 + length];
					buffer[0] = value->_type, buffer[1] = length;
					memcpy(&buffer[2], value->vString, length);
					stream->write(buffer, 2 + length);
					break;
				}

			}
		}

		stream->write(0x7D);
		stream->write(0x7D);

	}

};

#endif