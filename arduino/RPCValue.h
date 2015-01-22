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


	RPCValue(): type(vType) { vType = Null; }
	RPCValue(const bool value): type(vType) { vType = Bool; vBool = value; }
	RPCValue(const float value): type(vType) { vType = Float; vFloat = value; }
	RPCValue(const int32_t value): type(vType) { vType = Int; vInt = value; }
	RPCValue(const char value[]): type(vType) { vType = String; vString = strdup(value); }

	RPCValue(const RPCValue* &value): type(vType) {
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

	const byte &type;
	enum {Null, Bool, Float, Int, String};


	bool getBool(const bool value = false) { return (vType == Bool ? vBool : value); }
	float getFloat(const float value = 0) { return (vType == Float ? vFloat : value); }
	int32_t getInt(const int32_t value = 0) { return (vType == Int ? vInt : value); }
	const char* getString(const char value[] = "") { return (vType == String ? vString : value); }

};

#endif