#ifndef RPCResponse_h
#define RPCResponse_h

#include "RPCRequest.h"

class RPCResponse: public RPCRequest {

public:

	void pushNull() { RPCRequest::pushNull(); }
	void pushBool(bool value) { RPCRequest::pushBool(value); }
	void pushFloat(float value) { RPCRequest::pushFloat(value); }
	void pushInt(int32_t value) { RPCRequest::pushInt(value); }
	void pushString(char* value) { RPCRequest::pushString(value); }
	void pushValue(RPCValue* value) { RPCRequest::pushValue(value); }

	void unshiftNull() { RPCRequest::unshiftNull(); }
	void unshiftBool(bool value) { RPCRequest::unshiftBool(value); }
	void unshiftFloat(float value) { RPCRequest::unshiftFloat(value); }
	void unshiftInt(int32_t value) { RPCRequest::unshiftInt(value); }
	void unshiftString(char* value) { RPCRequest::unshiftString(value); }
	void unshiftValue(RPCValue* value) { RPCRequest::unshiftValue(value); }

};

#endif