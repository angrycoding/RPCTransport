#ifndef RPCValue_h
#define RPCValue_h

class RPCValue {

friend class RPCRequest;

private:

	byte vType;

	union {
		bool vBool;
		float vFloat;
		int32_t vInt;
		char* vString;
	};

	RPCValue() { vType = Null; }
	RPCValue(bool value) { vType = Bool; vBool = value; }
	RPCValue(float value) { vType = Float; vFloat = value; }
	RPCValue(int32_t value) { vType = Int; vInt = value; }
	RPCValue(char value[]) { vType = String; vString = strdup(value); }

	RPCValue(const RPCValue* &value) {
		if ((vType = value->vType) == String)
			vString = strdup(value->vString);
		else vInt = value->vInt;
	}

	~RPCValue() { if (vType == String) delete vString; }

	void write(Stream* stream) {

		switch (vType) {

			case Null: {
				stream->write(vType);
				break;
			}

			case Bool: {
				byte buffer[2] = {vType, vBool ? 1 : 0};
				stream->write(buffer, 2);
				break;
			}

			case Float: {
				byte buffer[5] = {vType};
				*reinterpret_cast<float*>(&buffer[1]) = vFloat;
				stream->write(buffer, 5);
				break;
			}

			case Int: {
				byte buffer[5] = {vType};
				*reinterpret_cast<int32_t*>(&buffer[1]) = vInt;
				stream->write(buffer, 5);
				break;
			}

			case String: {
				byte length = strlen(vString);
				byte buffer[2 + length];
				buffer[0] = vType, buffer[1] = length;
				memcpy(&buffer[2], vString, length);
				stream->write(buffer, 2 + length);
				break;
			}

		}
	}

public:

	enum {Null = 0, Bool = 1, Float = 2, Int = 3, String = 4};
	byte getType() const { return vType; }
	bool getType(byte type) const { return vType == type; }
	bool getBool(bool value = false) const { return (vType == Bool ? vBool : value); }
	float getFloat(float value = 0) const { return (vType == Float ? vFloat : value); }
	int32_t getInt(int32_t value = 0) const { return (vType == Int ? vInt : value); }
	const char* getString(const char value[] = "") const { return (vType == String ? vString : value); }

};

#endif