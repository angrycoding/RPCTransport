#ifndef RPCTransport_h
#define RPCTransport_h

#define RPC_NULL 0
#define RPC_BOOL 1
#define RPC_FLOAT 2
#define RPC_INT 3
#define RPC_STRING 4
#define RPC_START 5
#define RPC_ARGUMENTS 6
#define RPC_ARGUMENT_START 7
#define RPC_ARGUMENT_END 8
#define RPC_END 9

#define RPC_CMD_CALL 0x10
#define RPC_CMD_RET 0x20
#define RPC_CMD_BIND 0x30
#define RPC_CMD_READY 0x40

#define RPC_MAX_ARGS 16

#include "RPCPacket.h"


class RPCTransport: private RPCPacket {

	private:

		typedef void(*Handler)(RPCPacket*);
		typedef void(*BindHandler)(void);
		Handler handlers[5];
		byte handlerIndex;
		Stream* stream;
		BindHandler bindHandlers;
		bool canBindHandlers;


		byte processPacket(RPCPacket* packet) {
			byte command = 0;
			if (packet->read(stream)) {

				command = packet->shiftValue().getInt(0);

				if (command == RPC_CMD_READY) {
					begin(bindHandlers);
				}

				else if (command == RPC_CMD_CALL) {
					byte handlerIndex = packet->shiftValue().getInt(1);
					byte resultIndex = packet->shiftValue().getInt(2);
					handlers[handlerIndex](packet);
					packet->unshiftInt(resultIndex);
					packet->unshiftInt(RPC_CMD_RET);
					packet->write(stream);
				}

			}
			return command;
		}

	public:

		RPCTransport(Stream* serial): stream(serial), handlerIndex(0), bindHandlers(NULL), canBindHandlers(false) {}

		void begin(BindHandler handler) {

			while (handlerIndex)
				handlers[--handlerIndex] = NULL;

			if (handler != NULL) {
				canBindHandlers = true;
				(bindHandlers = handler)();
				canBindHandlers = false;
			}

			RPCPacket request;
			request.pushInt(RPC_CMD_READY);
			request.write(stream);


		}

		void process() { processPacket(this); }

		void on(const char value[], Handler handler) {
			if (canBindHandlers) {
				handlers[handlerIndex] = handler;
				RPCPacket request;
				request.reserve(3);
				request.pushInt(RPC_CMD_BIND);
				request.pushString(value);
				request.pushInt(handlerIndex++);
				request.write(stream);
			}
		}

		RPCPacket call(RPCValue args[], byte count) {
			RPCPacket request;
			request.reserve(count);
			for (byte c = 0; c < count; ++c) request.pushValue(args[c]);
			request.unshiftInt(RPC_CMD_CALL), request.write(stream);
			while (processPacket(&request) != RPC_CMD_RET);
			return request;
		}

};

#undef RPC_NULL
#undef RPC_BOOL
#undef RPC_FLOAT
#undef RPC_INT
#undef RPC_STRING
#undef RPC_START
#undef RPC_ARGUMENTS
#undef RPC_ARGUMENT
#undef RPC_END

#undef RPC_CMD_CALL
#undef RPC_CMD_RET
#undef RPC_CMD_BIND
#undef RPC_CMD_READY

#define RPCPacket(transport, ...) (((RPCTransport&)transport).call((RPCValue[]){__VA_ARGS__}, strlen(#__VA_ARGS__) ? RPCPacket_Num_Args(__VA_ARGS__) : 0));
#define RPCPacket_Num_Args(...) RPCPacket_Num_Args_Impl(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)
#define RPCPacket_Num_Args_Impl(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,N,...) N

#endif