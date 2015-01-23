#ifndef RPCValue_h
#define RPCValue_h

class RPCValue {

	friend class RPCRequest;
	friend class RPCTransport;

	private:

		byte vType;
		enum {Null = 0, Bool = 1, Float = 2, Int = 3, String = 4};
		union {bool vBool; float vFloat; int32_t vInt; char* vString;};

	public:

		RPCValue() { vType = Null; }
		RPCValue(bool value) { vType = Bool; vBool = value; }
		RPCValue(float value) { vType = Float; vFloat = value; }
		RPCValue(double value) { vType = Float; vFloat = (float)value; }
		RPCValue(int16_t value) { vType = Int; vInt = (int32_t)value;  }
		RPCValue(int32_t value) { vType = Int; vInt = value;  }
		RPCValue(char value[]) { vType = String; vString = strdup(value); }

		RPCValue(const RPCValue* &value) {
			if ((vType = value->vType) == String)
				vString = strdup(value->vString);
			else vInt = value->vInt;
		}

		RPCValue(const RPCValue &value) {
			if ((vType = value.vType) == String)
				vString = strdup(value.vString);
			else vInt = value.vInt;
		}

		~RPCValue() { if (vType == String) delete vString; }

		byte getType() const { return vType; }
		bool getType(byte type) const { return vType == type; }
		bool getBool(bool value = false) const { return (vType == Bool ? vBool : value); }
		float getFloat(float value = 0) const { return (vType == Float ? vFloat : value); }
		int32_t getInt(int32_t value = 0) const { return (vType == Int ? vInt : value); }
		const char* getString(const char value[] = "") const { return (vType == String ? vString : value); }

};

#endif