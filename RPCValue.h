#ifndef RPCValue_h
#define RPCValue_h

class RPCValue {

friend class RPCRequest;

private:

	byte _type;

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

	void write(Stream* stream) {

		switch (_type) {

			case RPCValue::Null: {
				stream->write(_type);
				break;
			}

			case RPCValue::Bool: {
				byte buffer[2] = {_type, vBool ? 1 : 0};
				stream->write(buffer, 2);
				break;
			}

			case RPCValue::Float: {
				byte buffer[5] = {_type};
				*reinterpret_cast<float*>(&buffer[1]) = vFloat;
				stream->write(buffer, 5);
				break;
			}

			case RPCValue::Int: {
				byte buffer[5] = {_type};
				*reinterpret_cast<int32_t*>(&buffer[1]) = vInt;
				stream->write(buffer, 5);
				break;
			}

			case RPCValue::String: {
				byte length = strlen(vString);
				byte buffer[2 + length];
				buffer[0] = _type, buffer[1] = length;
				memcpy(&buffer[2], vString, length);
				stream->write(buffer, 2 + length);
				break;
			}

		}
	}

public:

	union {
		bool vBool;
		float vFloat;
		int32_t vInt;
		char* vString;
	};

	const byte &type;
	enum {Null, Bool, Float, Int, String};


};

#endif