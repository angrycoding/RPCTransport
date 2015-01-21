#ifndef RPCValue_h
#define RPCValue_h

#include "RPCTransport.h"

class RPCValue {

friend class RPCTransport;

private:

	byte _type;

public:

	union {
		bool vBool;
		float vFloat;
		int32_t vInt;
		char* vString;
	};

	const byte &type;
	enum {Null, Bool, Float, Int, String};


	RPCValue(RPCValue* value): type(_type) {
		if ((_type = value->_type) == String)
			vString = strdup(value->vString);
		else vInt = value->vInt;
	}

	RPCValue(): type(_type) { _type = Null; }
	RPCValue(bool value): type(_type) { _type = Bool; vBool = value; }
	RPCValue(float value): type(_type) { _type = Float; vFloat = value; }
	RPCValue(int32_t value): type(_type) { _type = Int; vInt = value; }
	RPCValue(char* value): type(_type) { _type = String; vString = strdup(value); }
	~RPCValue() { if (_type == String) delete vString; }

};

#endif