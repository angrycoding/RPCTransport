#ifndef RPCRequest_h
#define RPCRequest_h

class RPCRequest {

friend class RPCTransport;
friend class RPCResponse;

private:

	byte count;
	byte reserved;
	RPCValue** arguments;
	static RPCValue nullValue;

	RPCRequest(): length(count) { count = 0; reserved = 0; arguments = NULL; }
	~RPCRequest() { clear(); }

	void reserve(byte size) {
		if (size <= count) return;
		RPCValue** buffer = new RPCValue*[reserved = size];
		for (byte i = 0; i < count; ++i)
			buffer[i] = arguments[i];
		delete[] arguments;
		arguments = buffer;
	}

	void push(RPCValue* value) {
		bool alloc = (reserved < ++count);
		RPCValue** buffer = (alloc ? new RPCValue*[count] : arguments);
		if (alloc) for (byte i = 0; i < count - 1; ++i) buffer[i] = arguments[i];
		buffer[count - 1] = value;
		if (alloc) delete[] arguments, arguments = buffer;
	}

	void unshift(RPCValue* value) {
		bool alloc = (reserved < ++count);
		RPCValue** buffer = (alloc ? new RPCValue*[count] : arguments);
		for (byte i = count - 1; i > 0; --i) buffer[i] = arguments[i - 1];
		buffer[0] = value;
		if (alloc) delete[] arguments, arguments = buffer;
	}

	void clear() {
		while (count--) delete arguments[count];
		delete[] arguments;
		arguments = NULL;
		reserved = 0;
		count = 0;
	}

	void write(Stream* stream) {
		stream->write(count);
		for (byte c = 0; c < count; ++c) {
			arguments[c]->write(stream);
		}
	}

protected:

	void pushNull() { push(new RPCValue()); }
	void pushBool(bool value) { push(new RPCValue(value)); }
	void pushFloat(float value) { push(new RPCValue(value)); }
	void pushInt(int32_t value) { push(new RPCValue(value)); }
	void pushString(char* value) { push(new RPCValue(value)); }
	void pushValue(const RPCValue* value) { push(new RPCValue(value)); }
	void unshiftNull() { unshift(new RPCValue()); }
	void unshiftBool(bool value) { unshift(new RPCValue(value)); }
	void unshiftFloat(float value) { unshift(new RPCValue(value)); }
	void unshiftInt(int32_t value) { unshift(new RPCValue(value)); }
	void unshiftString(char* value) { unshift(new RPCValue(value)); }
	void unshiftValue(const RPCValue* value) { unshift(new RPCValue(value)); }

public:

	const byte &length;
	byte getType(byte index) { return (index < count ? arguments[index]->vType : RPCValue::Null); }
	bool getType(byte index, byte type) { return (index < count ? arguments[index]->vType : RPCValue::Null) == type; }
	bool getBool(byte index, bool value = false) { return (index < count ? arguments[index]->getBool(value) : value); }
	float getFloat(byte index, float value = 0) { return (index < count ? arguments[index]->getFloat(value) : value); }
	int32_t getInt(byte index, int32_t value = 0) { return (index < count ? arguments[index]->getInt(value) : value); }
	const char* getString(byte index, const char value[] = "") { return (index < count ? arguments[index]->getString(value) : value); }
	const RPCValue* getValue(byte index) { return (index < count ? arguments[index] : &nullValue); }

};

RPCValue RPCRequest::nullValue;

#endif