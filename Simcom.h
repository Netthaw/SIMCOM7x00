#include <Arduino.h>

struct HTTPReqHeader
{
    String header;
    String value;
};

struct HTTPReqStruct
{
    String host;
    String url;
    String port;
    String method;
    String body = "";
};

struct MQTTStruct
{
    char protocolName[10] = "MQTT";
    char host[55]
    char port[10] = "1883";
    char clientID[20] = "ESP-SIMCOM";
    char username[30] = "admin";
    char password[30] = "123";
    char lvl = 0x03;
    char flags = 0xc2;
    unsigned int keepAlive = 60;
    char qos = 0x00;
    char packageID = 0x0001;
};

class Simcom
{
public:
    Simcom();
    void begin(unsigned long baudrate);
    void setTimeout(int interval);
    bool status();
    bool getIpDevice();
    bool setCIPMode(String mode);
    bool activePNP(String profile);
    bool closeNet();
    bool openNet();
    bool connectTCP(String ip, String port);
    bool connectMQTTSecure(MQTTStruct req);
    bool connectMQTT(MQTTStruct req);
    bool requestHTTP(HTTPReqStruct req, HTTPReqHeader header[], int size, String &out);

private:
    int _interval = 10 * 1000;
    bool readSerial(String wakeword, String &out);
    bool endRequestHTTP();
};