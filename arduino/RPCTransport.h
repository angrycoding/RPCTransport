#ifndef RPCTransport_h
#define RPCTransport_h

#define RPC_COMMAND_BIND 0x10
#define RPC_COMMAND_READY 0x20
#define RPC_COMMAND_RET 0x30
#define RPC_COMMAND_CALL 0x40
#define RPC_COMMAND_NOTIFY 0x50

#define RPC_STATE_IDLE 0
#define RPC_STATE_STARTING 1
#define RPC_STATE_RECEIVING 2
#define RPC_STATE_HANDLING 3


#include "RPCRequest.h"


class RPCTransport: private RPCRequest {

	private:

		byte transportState;
		byte handlerCount;
		typedef void(*Handler)(RPCRequest*);
		typedef void(*BindHandler)(void);

		Handler handlers[5];
		Stream* stream;
		BindHandler bindHandlers;



		byte processPacket(RPCRequest* packet) {

			if (!packet->read(stream)) return 0;
			byte command = packet->shiftValue().getInt(0);

			if (command >= RPC_COMMAND_CALL) {
				byte resultIndex, handlerIndex = packet->shiftValue().getInt(handlerCount);
				if (RPC_COMMAND_CALL == command) resultIndex = packet->shiftValue().getInt();
				if (handlerIndex < handlerCount) {
					byte oldTransportState = transportState;
					transportState = RPC_STATE_HANDLING;
					handlers[handlerIndex](packet);
					transportState = oldTransportState;
				}
				if (RPC_COMMAND_CALL == command) {
					packet->unshiftInt(resultIndex);
					packet->unshiftInt(RPC_COMMAND_RET);
					packet->write(stream);
				}
			}

			else if (RPC_COMMAND_READY == command) {
				transportState = RPC_STATE_IDLE;
				begin(bindHandlers);
			}

			return command;
		}

	public:

		RPCTransport(Stream* serial):
			stream(serial),
			handlerCount(0),
			bindHandlers(NULL),
			transportState(RPC_STATE_IDLE) {}

		void begin(BindHandler handler) {
			if (RPC_STATE_IDLE == transportState) {
				transportState = RPC_STATE_STARTING;
				while (handlerCount) handlers[--handlerCount] = NULL;
				if (handler != NULL) (bindHandlers = handler)();
				clear(), pushInt(RPC_COMMAND_READY), write(stream);
				transportState = RPC_STATE_RECEIVING;
			}
		}

		void on(const char value[], Handler handler) {
			if (RPC_STATE_STARTING == transportState && handler != NULL) {
				clear(), reserve(3);
				pushInt(RPC_COMMAND_BIND);
				pushString(value);
				pushInt(handlerCount);
				write(stream);
				handlers[handlerCount++] = handler;
			}
		}

		void process() {
			if (RPC_STATE_RECEIVING == transportState) {
				processPacket(this);
			}
		}

		RPCRequest call(RPCValue args[], byte count) {
			RPCRequest request;
			if (transportState >= RPC_STATE_RECEIVING) {
				while (state != RPC_START) processPacket(this);
				request.reserve(count);
				for (byte c = 0; c < count; ++c) request.pushValue(args[c]);
				request.unshiftInt(RPC_COMMAND_CALL), request.write(stream);
				while (processPacket(&request) != RPC_COMMAND_RET);
			}
			return request;
		}

		void notify(RPCValue args[], byte count) {
			if (transportState >= RPC_STATE_RECEIVING) {
				RPCRequest request; request.reserve(count);
				for (byte c = 0; c < count; ++c) request.pushValue(args[c]);
				request.unshiftInt(RPC_COMMAND_NOTIFY), request.write(stream);
			}
		}

};



#define RPCRequest(transport, ...) (((RPCTransport&)transport).call((RPCValue[]){__VA_ARGS__}, strlen(#__VA_ARGS__) ? RPC_NUM_ARGS(__VA_ARGS__) : 0));
#define RPCNotify(transport, ...) (((RPCTransport&)transport).notify((RPCValue[]){__VA_ARGS__}, strlen(#__VA_ARGS__) ? RPC_NUM_ARGS(__VA_ARGS__) : 0));
#define RPC_NUM_ARGS(...) RPC_NUM_ARGS_IMPL(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)
#define RPC_NUM_ARGS_IMPL(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,N,...) N

#endif