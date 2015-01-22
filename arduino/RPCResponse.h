#ifndef RPCResponse_h
#define RPCResponse_h

class RPCResponse: public RPCRequest {

friend class RPCTransport;

private:

	RPCResponse(): RPCRequest() {}

public:

	void pushNull() { RPCRequest::pushNull(); }
	void pushBool(bool value) { RPCRequest::pushBool(value); }
	void pushFloat(float value) { RPCRequest::pushFloat(value); }
	void pushInt(int32_t value) { RPCRequest::pushInt(value); }
	void pushString(char* value) { RPCRequest::pushString(value); }
	void pushValue(const RPCValue* value) { RPCRequest::pushValue(value); }
	void unshiftNull() { RPCRequest::unshiftNull(); }
	void unshiftBool(bool value) { RPCRequest::unshiftBool(value); }
	void unshiftFloat(float value) { RPCRequest::unshiftFloat(value); }
	void unshiftInt(int32_t value) { RPCRequest::unshiftInt(value); }
	void unshiftString(char* value) { RPCRequest::unshiftString(value); }
	void unshiftValue(const RPCValue* value) { RPCRequest::unshiftValue(value); }

};

#endif