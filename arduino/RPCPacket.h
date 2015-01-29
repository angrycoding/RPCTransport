#ifndef RPCPacket_h
#define RPCPacket_h

#include "RPCValue.h"

class RPCPacket {

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
			}
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
							state = RPC_ARGUMENT;
						} else state = RPC_START;
					} else state = RPC_END;
					goto start;
				}

				if (RPC_ARGUMENT == state) {
					if (stream->peek() < RPC_START) {
						state = stream->read();
						if (RPC_NULL == state) {
							pushNull();
							state = (count < argCount ? RPC_ARGUMENT : RPC_END);
						}
					} else state = RPC_START;
					goto start;
				}

				if (RPC_BOOL == state) {
					pushBool(!!stream->read());
					state = (count < argCount ? RPC_ARGUMENT : RPC_END);
					goto start;
				}

				if (RPC_FLOAT == state) {
					if (size < 4) return false;
					char buffer[4];
					stream->readBytes(buffer, 4);
					pushFloat(*reinterpret_cast<float*>(&buffer));
					state = (count < argCount ? RPC_ARGUMENT : RPC_END);
					goto start;
				}

				if (RPC_INT == state) {
					if (size < 4) return false;
					char buffer[4];
					stream->readBytes(buffer, 4);
					pushInt(*reinterpret_cast<int32_t*>(&buffer));
					state = (count < argCount ? RPC_ARGUMENT : RPC_END);
					goto start;
				}

				if (RPC_STRING == state) {
					if (size - 1 < stream->peek()) return false;
					char value[(size = stream->read()) + 1];
					stream->readBytes(value, size);
					value[size] = '\0';
					pushString(value);
					state = (count < argCount ? RPC_ARGUMENT : RPC_END);
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
			byte c = 0, type;
			stream->write((byte[]){0x7B, 0x7B, count}, 3);

			writeArgument: if (c < count) {

				value = values[c++];
				type = value->getType();


				if (RPC_NULL == type) {
					stream->write((byte)RPC_NULL);
					goto writeArgument;
				}

				if (RPC_BOOL == type) {
					stream->write((byte[]){RPC_BOOL, value->getBool() ? 1 : 0}, 2);
					goto writeArgument;
				}

				if (RPC_FLOAT == type) {
					byte buffer[5] = {RPC_FLOAT};
					*reinterpret_cast<float*>(&buffer[1]) = value->getFloat();
					stream->write(buffer, 5);
					goto writeArgument;
				}

				if (RPC_INT == type) {
					byte buffer[5] = {RPC_INT};
					*reinterpret_cast<int32_t*>(&buffer[1]) = value->getInt();
					stream->write(buffer, 5);
					goto writeArgument;
				}

				if (RPC_STRING == type) {
					const char* string = value->getString();
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
		void pushInt(const int32_t value) { push(new RPCValue(value)); }
		void pushString(const char value[]) { push(new RPCValue(value)); }
		void pushValue(const RPCValue* value) { push(new RPCValue(value)); }
		void pushValue(const RPCValue& value) { push(new RPCValue(value)); }

		void unshiftNull() { unshift(new RPCValue()); }
		void unshiftBool(const bool value) { unshift(new RPCValue(value)); }
		void unshiftFloat(const float value) { unshift(new RPCValue(value)); }
		void unshiftInt(const int32_t value) { unshift(new RPCValue(value)); }
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

		byte getType(byte index) { return (index < count ? values[index]->getType() : RPC_NULL); }
		bool getType(byte index, byte type) { return (index < count ? values[index]->getType(type) : RPC_NULL == type); }
		bool getBool(byte index, bool value = false) { return (index < count ? values[index]->getBool(value) : value); }
		float getFloat(byte index, float value = 0) { return (index < count ? values[index]->getFloat(value) : value); }
		int32_t getInt(byte index, int32_t value = 0) { return (index < count ? values[index]->getInt(value) : value); }
		const char* getString(byte index, const char value[] = "") { return (index < count ? values[index]->getString(value) : value); }
		const RPCValue* getValue(byte index) { return (index < count ? values[index] : &nullValue); }

		RPCPacket(): state(RPC_START), argCount(0), count(0), reserved(0), values(NULL), length(count) {}
		~RPCPacket() { clear(); }

};

RPCValue RPCPacket::nullValue;

#endif