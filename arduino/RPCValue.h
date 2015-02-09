#ifndef RPCValue_h
#define RPCValue_h

class RPCValue {

	friend class RPCRequest;

	private:

		uint8_t vType;

		union {
			bool vBool;
			float vFloat;
			int8_t vInt8;
			int16_t vInt16;
			int32_t vInt32;
			uint8_t vUInt8;
			uint16_t vUInt16;
			uint32_t vUInt32;
			char* vString;
		};

	public:

		RPCValue() { vType = RPC_NULL; }
		RPCValue(bool value) { vType = RPC_BOOL; vBool = value; }
		RPCValue(float value) { vType = RPC_FLOAT; vFloat = value; }
		RPCValue(double value) { vType = RPC_FLOAT; vFloat = (float)value; }
		RPCValue(int8_t value) { vType = RPC_INT8; vInt8 = value; }
		RPCValue(int16_t value) { vType = RPC_INT16; vInt16 = value; }
		RPCValue(int32_t value) { vType = RPC_INT32; vInt32 = value; }
		RPCValue(uint8_t value) { vType = RPC_UINT8; vUInt8 = value; }
		RPCValue(uint16_t value) { vType = RPC_UINT16; vUInt16 = value; }
		RPCValue(uint32_t value) { vType = RPC_UINT32; vUInt32 = value; }
		RPCValue(const char value[]) { vType = RPC_STRING; vString = strdup(value); }

		RPCValue(const RPCValue* value) {
			if ((vType = value->vType) == RPC_STRING)
				vString = strdup(value->vString);
			else vInt32 = value->vInt32;
		}

		RPCValue(const RPCValue &value) {
			if ((vType = value.vType) == RPC_STRING)
				vString = strdup(value.vString);
			else vInt32 = value.vInt32;
		}

		~RPCValue() { if (vType == RPC_STRING) delete vString; }

		uint8_t getType() const { return vType; }
		bool getType(uint8_t type) const { return vType == type; }
		bool getBool(bool value = false) const { return (vType == RPC_BOOL ? vBool : value); }
		float getFloat(float value = 0) const { return (vType == RPC_FLOAT ? vFloat : value); }
		int8_t getInt8(int8_t value = 0) const { return (vType > RPC_FLOAT && vType < RPC_STRING && (int16_t)vUInt32 == vUInt32 ? vUInt32 : value); }
		int16_t getInt16(int16_t value = 0) const { return (vType > RPC_FLOAT && vType < RPC_STRING && (int16_t)vUInt32 == vUInt32 ? vUInt32 : value); }
		int32_t getInt32(int32_t value = 0) const { return (vType > RPC_FLOAT && vType < RPC_STRING && (int32_t)vUInt32 == vUInt32 ? vUInt32 : value); }
		uint8_t getUInt8(uint8_t value = 0) const { return (vType > RPC_FLOAT && vType < RPC_STRING && (uint8_t)vUInt32 == vUInt32 ? vUInt32 : value); }
		uint16_t getUInt16(uint16_t value = 0) const { return (vType > RPC_FLOAT && vType < RPC_STRING && (uint16_t)vUInt32 == vUInt32 ? vUInt32 : value); }
		uint32_t getUInt32(uint32_t value = 0) const { return (vType > RPC_FLOAT && vType < RPC_STRING && (uint32_t)vUInt32 == vUInt32 ? vUInt32 : value); }
		const char* getString(const char value[] = "") const { return (vType == RPC_STRING ? vString : value); }

};

#endif
