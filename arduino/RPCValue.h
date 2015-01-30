#ifndef RPCValue_h
#define RPCValue_h

class RPCValue {

	friend class RPCRequest;

	private:

		byte vType;
		union {bool vBool; float vFloat; int32_t vInt; char* vString;};

	public:

		RPCValue() { vType = RPC_NULL; }
		RPCValue(bool value) { vType = RPC_BOOL; vBool = value; }
		RPCValue(float value) { vType = RPC_FLOAT; vFloat = value; }
		RPCValue(double value) { vType = RPC_FLOAT; vFloat = (float)value; }
		RPCValue(int16_t value) { vType = RPC_INT; vInt = (int32_t)value; }
		RPCValue(int32_t value) { vType = RPC_INT; vInt = value; }
		RPCValue(const char value[]) { vType = RPC_STRING; vString = strdup(value); }

		RPCValue(const RPCValue* value) {
			if ((vType = value->vType) == RPC_STRING)
				vString = strdup(value->vString);
			else vInt = value->vInt;
		}

		RPCValue(const RPCValue &value) {
			if ((vType = value.vType) == RPC_STRING)
				vString = strdup(value.vString);
			else vInt = value.vInt;
		}

		~RPCValue() { if (vType == RPC_STRING) delete vString; }

		byte getType() const { return vType; }
		bool getType(byte type) const { return vType == type; }
		bool getBool(bool value = false) const { return (vType == RPC_BOOL ? vBool : value); }
		float getFloat(float value = 0) const { return (vType == RPC_FLOAT ? vFloat : value); }
		int32_t getInt(int32_t value = 0) const { return (vType == RPC_INT ? vInt : value); }
		const char* getString(const char value[] = "") const { return (vType == RPC_STRING ? vString : value); }

};

#endif
