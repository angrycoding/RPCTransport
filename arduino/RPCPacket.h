#ifndef RPCPacket_h
#define RPCPacket_h

#include "RPCValue.h"

class RPCPacket {

	friend class RPCTransport;

	private:

		byte state;
		byte count;
		byte argCount;
		byte reserved;
		RPCValue** values;
		static RPCValue nullValue;

		void push(RPCValue* value) {
			bool alloc = (reserved < ++count);
			RPCValue** buffer = (alloc ? new RPCValue*[count] : values);
			if (alloc) for (byte i = 0; i < count - 1; ++i) buffer[i] = values[i];
			buffer[count - 1] = value;
			if (alloc) delete[] values, values = buffer;
		}

		void unshift(RPCValue* value) {
			bool alloc = (reserved < ++count);
			RPCValue** buffer = (alloc ? new RPCValue*[count] : values);
			for (byte i = count - 1; i > 0; --i) buffer[i] = values[i - 1];
			buffer[0] = value;
			if (alloc) delete[] values, values = buffer;
		}

		void reserve(byte size) {
			if (size <= count) return;
			RPCValue** buffer = new RPCValue*[reserved = size];
			for (byte i = 0; i < count; ++i)
				buffer[i] = values[i];
			delete[] values;
			values = buffer;
		}

		bool read(Stream* stream) {
			byte size;
			while (size = stream->available()) switch (state) {

				case RPC_START: {
					if (size < 2) return false;
					if (stream->read() == 0x7B && stream->read() == 0x7B)
						state = RPC_ARGUMENTS;
					break;
				}

				case RPC_ARGUMENTS: {
					if (clear(), argCount = stream->read()) {
						if (argCount < RPC_MAX_ARGS) {
							reserve(argCount);
							state = RPC_ARGUMENT;
						} else state = RPC_START;
					} else state = RPC_END;
					break;
				}

				case RPC_ARGUMENT: {
					if (stream->peek() < RPC_START) {
						state = stream->read();
						if (RPC_NULL == state) {
							pushNull();
							state = (count < argCount ? RPC_ARGUMENT : RPC_END);
						}
					} else state = RPC_START;
					break;
				}

				case RPC_BOOL: {
					pushBool(!!stream->read());
					state = (count < argCount ? RPC_ARGUMENT : RPC_END);
					break;
				}

				case RPC_FLOAT: {
					if (size < 4) return false;
					char buffer[4];
					stream->readBytes(buffer, 4);
					pushFloat(*reinterpret_cast<float*>(&buffer));
					state = (count < argCount ? RPC_ARGUMENT : RPC_END);
					break;
				}

				case RPC_INT: {
					if (size < 4) return false;
					char buffer[4];
					stream->readBytes(buffer, 4);
					pushInt(*reinterpret_cast<int32_t*>(&buffer));
					state = (count < argCount ? RPC_ARGUMENT : RPC_END);
					break;
				}

				case RPC_STRING: {
					if (size - 1 < stream->peek()) return false;
					char value[(size = stream->read()) + 1];
					stream->readBytes(value, size);
					value[size] = '\0';
					pushString(value);
					state = (count < argCount ? RPC_ARGUMENT : RPC_END);
					break;
				}

				case RPC_END: {
					if (size < 2) return false;
					state = RPC_START;
					if (stream->peek() == 0x7D) stream->read(); else break;
					if (stream->peek() == 0x7D) stream->read(); else break;

					return true;

				}
			}

			return false;
		}

		void write(Stream* stream) {
			byte c;
			RPCValue* value;
			stream->write((byte[]){0x7B, 0x7B, count}, 3);
			for (c = 0; c < count; ++c) {
				switch ((value = values[c])->getType()) {

					case RPC_NULL: {
						stream->write((byte)RPC_NULL);
						break;
					}

					case RPC_BOOL: {
						stream->write((byte[]){RPC_BOOL, value->getBool() ? 1 : 0}, 2);
						break;
					}

					case RPC_FLOAT: {
						byte buffer[5] = {RPC_FLOAT};
						*reinterpret_cast<float*>(&buffer[1]) = value->getFloat();
						stream->write(buffer, 5);
						break;
					}

					case RPC_INT: {
						byte buffer[5] = {RPC_INT};
						*reinterpret_cast<int32_t*>(&buffer[1]) = value->getInt();
						stream->write(buffer, 5);
						break;
					}

					case RPC_STRING: {
						byte length = strlen(value->getString());
						byte buffer[2 + length];
						buffer[0] = RPC_STRING, buffer[1] = length;
						memcpy(&buffer[2], value->getString(), length);
						stream->write(buffer, 2 + length);
						break;
					}

				}
			}
			stream->write((byte[]){0x7D, 0x7D}, 2);
			stream->flush();
		}

	public:

		const byte &length;

		void clear() {
			while (count--) delete values[count];
			delete[] values;
			values = NULL;
			reserved = 0;
			count = 0;
		}

		RPCPacket* remove(int8_t index) {

			if (index > 0) {

				RPCValue** buffer = new RPCValue*[count - index];
				for (byte i = 0; i < count; ++i) {
					if (i < index) delete values[i];
					else buffer[i - index] = values[i];
				}
				count-= index;
				delete [] values, values = buffer;

			}

			return this;

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

		byte getType(byte index) { return (index < count ? values[index]->getType() : RPC_NULL); }
		bool getType(byte index, byte type) { return (index < count ? values[index]->getType(type) : RPC_NULL == type); }
		bool getBool(byte index, bool value = false) { return (index < count ? values[index]->getBool(value) : value); }
		float getFloat(byte index, float value = 0) { return (index < count ? values[index]->getFloat(value) : value); }
		int32_t getInt(byte index, int32_t value = 0) { return (index < count ? values[index]->getInt(value) : value); }
		const char* getString(byte index, const char value[] = "") { return (index < count ? values[index]->getString(value) : value); }
		const RPCValue* getValue(byte index) { return (index < count ? values[index] : &nullValue); }

		RPCPacket(): length(count) {
			argCount = 0, state = RPC_START;
			count = 0, reserved = 0, values = NULL;
		}

		~RPCPacket() {
			clear();
		}

};

RPCValue RPCPacket::nullValue;

#endif