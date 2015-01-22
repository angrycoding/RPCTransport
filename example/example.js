var RPCTransport = require('../nodejs/RPCTransport.js');

var transport = new RPCTransport('/dev/cu.usbserial-A965DFR7', 115200);

transport.open();