#ifndef RPCRequest_h
#define RPCRequest_h

#define RPC_NULL 0
#define RPC_BOOL 1
#define RPC_FLOAT 2
#define RPC_INT8 3
#define RPC_INT16 4
#define RPC_INT32 5
#define RPC_UINT8 6
#define RPC_UINT16 7
#define RPC_UINT32 8
#define RPC_STRING 9
#define RPC_START 10
#define RPC_ARGUMENTS 11
#define RPC_ARGUMENT_START 12
#define RPC_ARGUMENT_END 13
#define RPC_END 14

#define RPC_MAX_ARGS 16

#include "RPCValue.h"

class RPCRequest {

	friend class RPCTransport;

	private:

		byte state;
		byte argCount;
		byte count;
		byte reserved;
		RPCValue** values;
		static RPCValue nullValue;

		void push(RPCValue* value) {
			if (reserved < ++count) {
				RPCValue** buffer = new RPCValue*[count];
				memcpy(buffer, values, (count - 1) * sizeof(RPCValue*));
				delete[] values, values = buffer;
			}
			values[count - 1] = value;
		}

		void unshift(RPCValue* value) {
			if (reserved < ++count) {
				RPCValue** buffer = new RPCValue*[count];
				memcpy(&buffer[1], values, (count - 1) * sizeof(RPCValue*));
				delete[] values, values = buffer;
			} else memmove(&values[1], values, (count - 1) * sizeof(RPCValue*));
			values[0] = value;
		}

		void reserve(byte size) {
			if (size <= count) return;
			RPCValue** buffer = new RPCValue*[reserved = size];
			memcpy(buffer, values, count * sizeof(RPCValue*));
			delete[] values, values = buffer;
		}

		bool read(Stream* stream) {

			byte size;

			start: if (size = stream->available()) {

				if (RPC_START == state) {
					if (size < 2) return false;
					if (stream->read() == 0x7B && stream->read() == 0x7B)
						state = RPC_ARGUMENTS;
					goto start;
				}

				if (RPC_ARGUMENTS == state) {
					if (clear(), argCount = stream->read()) {
						if (argCount < RPC_MAX_ARGS) {
							reserve(argCount);
							state = RPC_ARGUMENT_START;
						} else state = RPC_START;
					} else state = RPC_END;
					goto start;
				}

				if (RPC_ARGUMENT_START == state) {
					if (stream->peek() < RPC_START) {
						state = stream->read();
					} else state = RPC_START;
					goto start;
				}

				if (RPC_NULL == state) {
					pushNull();
					state = RPC_ARGUMENT_END;
					goto start;
				}

				if (RPC_BOOL == state) {
					pushBool(!!stream->read());
					state = RPC_ARGUMENT_END;
					goto start;
				}

				if (RPC_FLOAT == state) {
					if (size < 4) return false;
					char buffer[4];
					stream->readBytes(buffer, 4);
					pushFloat(*reinterpret_cast<float*>(&buffer));
					state = RPC_ARGUMENT_END;
					goto start;
				}

				if (RPC_INT8 == state) {
					char buffer[1];
					stream->readBytes(buffer, 1);
					pushInt8(*reinterpret_cast<int8_t*>(&buffer));
					state = RPC_ARGUMENT_END;
					goto start;
				}

				if (RPC_INT16 == state) {
					if (size < 2) return false;
					char buffer[2];
					stream->readBytes(buffer, 2);
					pushInt16(*reinterpret_cast<int16_t*>(&buffer));
					state = RPC_ARGUMENT_END;
					goto start;
				}

				if (RPC_INT32 == state) {
					if (size < 4) return false;
					char buffer[4];
					stream->readBytes(buffer, 4);
					pushInt32(*reinterpret_cast<int32_t*>(&buffer));
					state = RPC_ARGUMENT_END;
					goto start;
				}

				if (RPC_UINT8 == state) {
					char buffer[1];
					stream->readBytes(buffer, 1);
					pushUInt8(*reinterpret_cast<uint8_t*>(&buffer));
					state = RPC_ARGUMENT_END;
					goto start;
				}

				if (RPC_UINT16 == state) {
					if (size < 2) return false;
					char buffer[2];
					stream->readBytes(buffer, 2);
					pushUInt16(*reinterpret_cast<uint16_t*>(&buffer));
					state = RPC_ARGUMENT_END;
					goto start;
				}

				if (RPC_UINT32 == state) {
					if (size < 4) return false;
					char buffer[4];
					stream->readBytes(buffer, 4);
					pushUInt32(*reinterpret_cast<uint32_t*>(&buffer));
					state = RPC_ARGUMENT_END;
					goto start;
				}

				if (RPC_STRING == state) {
					if (size - 1 < stream->peek()) return false;
					char value[(size = stream->read()) + 1];
					stream->readBytes(value, size);
					value[size] = '\0';
					pushString(value);
					state = RPC_ARGUMENT_END;
					goto start;
				}

				if (RPC_ARGUMENT_END == state) {
					state = (count < argCount ? RPC_ARGUMENT_START : RPC_END);
					goto start;
				}

				if (RPC_END == state) {
					if (size < 2) return false;
					state = RPC_START;
					if (stream->peek() == 0x7D) stream->read(); else goto start;
					if (stream->peek() == 0x7D) stream->read(); else goto start;
					return true;
				}


			}

			return false;
		}

		void write(Stream* stream) {

			RPCValue* value;
			byte c = 0;
			stream->write((byte[]){0x7B, 0x7B, count}, 3);

			writeArgument: if (c < count) {

				value = values[c++];

				if (RPC_NULL == value->vType) {
					stream->write((byte)RPC_NULL);
					goto writeArgument;
				}

				if (RPC_BOOL == value->vType) {
					stream->write((byte[]){RPC_BOOL, value->vBool ? 1 : 0}, 2);
					goto writeArgument;
				}

				if (RPC_FLOAT == value->vType) {
					byte buffer[5] = {RPC_FLOAT};
					*reinterpret_cast<float*>(&buffer[1]) = value->vFloat;
					stream->write(buffer, 5);
					goto writeArgument;
				}

				if (RPC_INT8 == value->vType) {
					byte buffer[2] = {RPC_INT8};
					*reinterpret_cast<int8_t*>(&buffer[1]) = value->vInt8;
					stream->write(buffer, 2);
					goto writeArgument;
				}

				if (RPC_INT16 == value->vType) {
					byte buffer[3] = {RPC_INT16};
					*reinterpret_cast<int16_t*>(&buffer[1]) = value->vInt16;
					stream->write(buffer, 3);
					goto writeArgument;
				}

				if (RPC_INT32 == value->vType) {
					byte buffer[5] = {RPC_INT32};
					*reinterpret_cast<int32_t*>(&buffer[1]) = value->vInt32;
					stream->write(buffer, 5);
					goto writeArgument;
				}

				if (RPC_UINT8 == value->vType) {
					byte buffer[2] = {RPC_UINT8};
					*reinterpret_cast<uint8_t*>(&buffer[1]) = value->vUInt8;
					stream->write(buffer, 2);
					goto writeArgument;
				}

				if (RPC_UINT16 == value->vType) {
					byte buffer[3] = {RPC_UINT16};
					*reinterpret_cast<uint16_t*>(&buffer[1]) = value->vUInt16;
					stream->write(buffer, 3);
					goto writeArgument;
				}

				if (RPC_UINT32 == value->vType) {
					byte buffer[5] = {RPC_UINT32};
					*reinterpret_cast<uint32_t*>(&buffer[1]) = value->vUInt32;
					stream->write(buffer, 5);
					goto writeArgument;
				}

				if (RPC_STRING == value->vType) {
					const char* string = value->vString;
					byte length = strlen(string);
					byte buffer[2 + length];
					buffer[0] = RPC_STRING, buffer[1] = length;
					memcpy(&buffer[2], string, length);
					stream->write(buffer, 2 + length);
					goto writeArgument;
				}

			}

			stream->write((byte[]){0x7D, 0x7D}, 2);
			stream->flush();
		}

	public:

		const byte &length;

		void clear() {
			while (count)
				delete values[--count];
			delete[] values;
			values = NULL;
			reserved = 0;
		}

		void pushNull() { push(new RPCValue()); }
		void pushBool(const bool value) { push(new RPCValue(value)); }
		void pushFloat(const float value) { push(new RPCValue(value)); }
		void pushInt8(const int8_t value) { push(new RPCValue(value)); }
		void pushInt16(const int16_t value) { push(new RPCValue(value)); }
		void pushInt32(const int32_t value) { push(new RPCValue(value)); }
		void pushUInt8(const uint8_t value) { push(new RPCValue(value)); }
		void pushUInt16(const uint16_t value) { push(new RPCValue(value)); }
		void pushUInt32(const uint32_t value) { push(new RPCValue(value)); }
		void pushString(const char value[]) { push(new RPCValue(value)); }
		void pushValue(const RPCValue* value) { push(new RPCValue(value)); }
		void pushValue(const RPCValue& value) { push(new RPCValue(value)); }

		void unshiftNull() { unshift(new RPCValue()); }
		void unshiftBool(const bool value) { unshift(new RPCValue(value)); }
		void unshiftFloat(const float value) { unshift(new RPCValue(value)); }
		void unshiftInt8(const int8_t value) { unshift(new RPCValue(value)); }
		void unshiftInt16(const int16_t value) { unshift(new RPCValue(value)); }
		void unshiftInt32(const int32_t value) { unshift(new RPCValue(value)); }
		void unshiftUInt8(const uint8_t value) { unshift(new RPCValue(value)); }
		void unshiftUInt16(const uint16_t value) { unshift(new RPCValue(value)); }
		void unshiftUInt32(const uint32_t value) { unshift(new RPCValue(value)); }
		void unshiftString(const char value[]) { unshift(new RPCValue(value)); }
		void unshiftValue(const RPCValue* value) { unshift(new RPCValue(value)); }
		void unshiftValue(const RPCValue& value) { unshift(new RPCValue(value)); }

		RPCValue shiftValue() {
			if (!count) return nullValue;
			RPCValue** buffer = new RPCValue*[count -= 1];
			memcpy(buffer, &values[1], count * sizeof(RPCValue*));
			RPCValue result(values[0]);
			delete values[0];
			delete [] values, values = buffer;
			return result;
		}

		RPCValue popValue() {
			if (!count) return nullValue;
			RPCValue** buffer = new RPCValue*[count -= 1];
			memcpy(buffer, values, count * sizeof(RPCValue*));
			RPCValue result(values[count]);
			delete values[count];
			delete [] values, values = buffer;
			return result;
		}

		byte getType(byte index) { return (index < count ? values[index]->vType : RPC_NULL); }
		bool getType(byte index, byte type) { return (index < count ? values[index]->vType : RPC_NULL) == type; }
		bool getBool(byte index, bool value = false) { return (index < count ? values[index]->getBool(value) : value); }
		float getFloat(byte index, float value = 0) { return (index < count ? values[index]->getFloat(value) : value); }
		int8_t getInt8(byte index, int8_t value = 0) { return (index < count ? values[index]->getInt8(value) : value); }
		int16_t getInt16(byte index, int16_t value = 0) { return (index < count ? values[index]->getInt16(value) : value); }
		int32_t getInt32(byte index, int32_t value = 0) { return (index < count ? values[index]->getInt32(value) : value); }
		uint8_t getUInt8(byte index, uint8_t value = 0) { return (index < count ? values[index]->getUInt8(value) : value); }
		uint16_t getUInt16(byte index, uint16_t value = 0) { return (index < count ? values[index]->getUInt16(value) : value); }
		uint32_t getUInt32(byte index, uint32_t value = 0) { return (index < count ? values[index]->getUInt32(value) : value); }
		const char* getString(byte index, const char value[] = "") { return (index < count ? values[index]->getString(value) : value); }
		const RPCValue* getValue(byte index) { return (index < count ? values[index] : &nullValue); }

		RPCRequest(): state(RPC_START), argCount(0), count(0), reserved(0), values(NULL), length(count) {}
		~RPCRequest() { clear(); }

};

RPCValue RPCRequest::nullValue;

#endif