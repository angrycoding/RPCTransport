#ifndef RPCRequest_h
#define RPCRequest_h

class RPCRequest {

friend class RPCTransport;
friend class RPCResponse;

private:

	byte size;
	RPCValue** arguments;

	// add allocation capability (no need to fuck with array if request size is known)
	RPCRequest(): length(size) { size = 0; arguments = NULL; }
	~RPCRequest() { clear(); }

	void push(RPCValue* value) {
		RPCValue** newBuffer = new RPCValue*[++size];
		for (byte i = 0; i < size - 1; ++i)
			newBuffer[i] = arguments[i];
		newBuffer[size - 1] = value;
		delete[] arguments;
		arguments = newBuffer;
	}

	void unshift(RPCValue* value) {
		RPCValue** newBuffer = new RPCValue*[++size];
		for (byte i = size - 1; i > 0; --i)
			newBuffer[i] = arguments[i - 1];
		newBuffer[0] = value;
		delete[] arguments;
		arguments = newBuffer;
	}

	void clear() {
		while (size--) delete arguments[size];
		delete[] arguments;
		arguments = NULL;
		size = 0;
	}

	void write(Stream* stream) {
		stream->write(size);
		for (byte c = 0; c < size; c++) {
			arguments[c]->write(stream);
		}
	}

protected:

	void pushNull() { push(new RPCValue()); }
	void pushBool(bool value) { push(new RPCValue(value)); }
	void pushFloat(float value) { push(new RPCValue(value)); }
	void pushInt(int32_t value) { push(new RPCValue(value)); }
	void pushString(char* value) { push(new RPCValue(value)); }
	void pushValue(RPCValue* value) { push(new RPCValue(value)); }

	void unshiftNull() { unshift(new RPCValue()); }
	void unshiftBool(bool value) { unshift(new RPCValue(value)); }
	void unshiftFloat(float value) { unshift(new RPCValue(value)); }
	void unshiftInt(int32_t value) { unshift(new RPCValue(value)); }
	void unshiftString(char* value) { unshift(new RPCValue(value)); }
	void unshiftValue(RPCValue* value) { unshift(new RPCValue(value)); }

public:

	const byte &length;

	RPCValue* operator[](byte index) {
		return arguments[index];
	}


};



#endif