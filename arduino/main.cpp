#include <QDebug>
#include <QThread>

typedef unsigned char byte;



class RPCTransport {

public:

    byte serial;

    RPCTransport(byte serial) {
        this->serial = serial;
    }
};


class RPCRequest;

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

   void created() {
       qDebug() << "created" << vType;
   }

   void destroyed() {
       qDebug() << "destroyed" << vType;
   }

public:

    RPCValue() { vType = Null; created(); }
    RPCValue(void* x) { vType = Null; created(); }
    RPCValue(bool value) { vType = Bool; vBool = value; created(); }
    RPCValue(float value) { vType = Float; vFloat = value; created(); }
    RPCValue(int32_t value) { vType = Int; vInt = value; created(); }
    RPCValue(char value[]) { vType = String; vString = strdup(value); created(); }

    RPCValue(const RPCValue* &value) {
        if ((vType = value->vType) == String)
            vString = strdup(value->vString);
        else vInt = value->vInt;
    }

    ~RPCValue() { if (vType == String) delete vString; destroyed(); }

    enum {Null = 0, Bool = 1, Float = 2, Int = 3, String = 4};
    byte getType() const { return vType; }
    bool getType(byte type) const { return vType == type; }
    bool getBool(bool value = false) const { return (vType == Bool ? vBool : value); }
    float getFloat(float value = 0) const { return (vType == Float ? vFloat : value); }
    int32_t getInt(int32_t value = 0) const { return (vType == Int ? vInt : value); }
    const char* getString(const char value[] = "") const { return (vType == String ? vString : value); }

};


class RPCRequest {

private:

    byte count;
    byte reserved;
    RPCValue** arguments;
    static RPCValue nullValue;

    void reserve(byte size) {
        if (size <= count) return;
        RPCValue** buffer = new RPCValue*[reserved = size];
        for (byte i = 0; i < count; ++i)
            buffer[i] = arguments[i];
        delete[] arguments;
        arguments = buffer;
    }

    void push(RPCValue* value) {
        bool alloc = (reserved < ++count);
        RPCValue** buffer = (alloc ? new RPCValue*[count] : arguments);
        if (alloc) for (byte i = 0; i < count - 1; ++i) buffer[i] = arguments[i];
        buffer[count - 1] = value;
        if (alloc) delete[] arguments, arguments = buffer;
    }

    void unshift(RPCValue* value) {
        bool alloc = (reserved < ++count);
        RPCValue** buffer = (alloc ? new RPCValue*[count] : arguments);
        for (byte i = count - 1; i > 0; --i) buffer[i] = arguments[i - 1];
        buffer[0] = value;
        if (alloc) delete[] arguments, arguments = buffer;
    }

    void clear() {
        while (count--) delete arguments[count];
        delete[] arguments;
        arguments = NULL;
        reserved = 0;
        count = 0;
    }

public:

    RPCRequest(): length(count) {
        qDebug() << "RPCRequest";
        count = 0;
        reserved = 0;
        arguments = NULL;
    }


     ~RPCRequest() {
        clear();
        qDebug() << "~RPCRequest";
    }

    void pushNull() { push(new RPCValue()); }

    RPCRequest* pushBool(bool value) {
        push(new RPCValue(value));
        return this;
    }

    void pushFloat(float value) { push(new RPCValue(value)); }
    void pushInt(int32_t value) { push(new RPCValue(value)); }

    RPCRequest* pushString(char* value) {
        push(new RPCValue(value));
        return this;
    }

    void pushValue(const RPCValue* value) { push(new RPCValue(value)); }

    void unshiftNull() { unshift(new RPCValue()); }
    void unshiftBool(bool value) { unshift(new RPCValue(value)); }
    void unshiftFloat(float value) { unshift(new RPCValue(value)); }
    void unshiftInt(int32_t value) { unshift(new RPCValue(value)); }
    RPCRequest* unshiftString(char* value) { unshift(new RPCValue(value)); return this; }
    void unshiftValue(const RPCValue* value) { unshift(new RPCValue(value)); }

public:

    const byte &length;
    byte getType(byte index) { return (index < count ? arguments[index]->vType : RPCValue::Null); }
    bool getType(byte index, byte type) { return (index < count ? arguments[index]->vType : RPCValue::Null) == type; }
    bool getBool(byte index, bool value = false) { return (index < count ? arguments[index]->getBool(value) : value); }
    float getFloat(byte index, float value = 0) { return (index < count ? arguments[index]->getFloat(value) : value); }
    int32_t getInt(byte index, int32_t value = 0) { return (index < count ? arguments[index]->getInt(value) : value); }
    const char* getString(byte index, const char value[] = "") { return (index < count ? arguments[index]->getString(value) : value); }
    const RPCValue* getValue(byte index) { return (index < count ? arguments[index] : &nullValue); }

    RPCRequest(const RPCTransport &transport, RPCValue args[], byte argCount): length(count) {

        qDebug() << "RPCRequest argCount =" << argCount;
        count = 0;
        reserved = 0;
        arguments = NULL;

        for (byte c = 0; c < argCount; c++) {
            if (args[c].getType(RPCValue::Null)) {
                qDebug() << c << "Null";
            }
            else if (args[c].getType(RPCValue::Bool)) {
                qDebug() << c << "Bool" << args[c].getBool();
            }
            else if (args[c].getType(RPCValue::Float)) {
                qDebug() << c << "Float" << args[c].getFloat();
            }
            else if (args[c].getType(RPCValue::Int)) {
                qDebug() << c << "Int" << args[c].getInt();
            }
            else if (args[c].getType(RPCValue::String)) {
                qDebug() << c << "String" << args[c].getString();
            }
        }

        qDebug() << "write arguments...";
        qDebug() << "wait for response...";



        QObject().thread()->usleep(1000*1000*2);
        pushString("answer");
        pushInt(42);
    }


};

#define RPCRequest(transport, ...) RPCRequest(transport, (RPCValue[]){__VA_ARGS__}, strlen(#__VA_ARGS__) ? VA_NUM_ARGS(__VA_ARGS__) : 0)
#define VA_NUM_ARGS(...) VA_NUM_ARGS_IMPL(__VA_ARGS__, 5,4,3,2,1)
#define VA_NUM_ARGS_IMPL(_1,_2,_3,_4,_5,N,...) N

int main(int argc, char *argv[]) {

//    RPCValue value();

//    qDebug() << NULL;

    RPCTransport transport(10);
    RPCRequest x = RPCRequest(transport, "hello", true, 195, (float)3.14);
    qDebug() << x.getString(0);
    qDebug() << x.getInt(1);

//    qDebug() << RPCRequest(transport, (float)3.14);


}
