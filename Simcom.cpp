#include "Simcom.h"

Simcom::Simcom()
{
}

void Simcom::setTimeout(int interval)
{
    _interval = interval * 1000;
}

void Simcom::begin(unsigned long baudrate)
{
    Serial2.begin(baudrate, SERIAL_8N1, 16, 17);
}

bool Simcom::status()
{
    Serial2.println("AT");
    String out;
    if (readSerial("AT", out))
        if (out.indexOf("OK") > -1)
        {
            Serial.println(out);
            return true;
        }

    return false;
}

bool Simcom::openNet()
{
    Serial2.println("at+netopen");
    String out;
    if (readSerial("+NETOPEN: 0", out))
        if (out.indexOf("OK") > -1)
        {
            Serial.println(out);
            return true;
        }

    return false;
}

bool Simcom::closeNet()
{
    Serial2.println("at+netclose");
    String out;
    if (readSerial("+NETCLOSE: 0", out))
        if (out.indexOf("OK") > -1)
        {
            Serial.println(out);
            return true;
        }

    return false;
}

bool Simcom::activePNP(String profile)
{
    Serial2.println("at+csocksetpn=" + profile);
    String out;
    if (readSerial("OK", out))
    {
        Serial.println(out);
        return true;
    }

    return false;
}

bool Simcom::setCIPMode(String mode)
{
    Serial2.println("at+cipmode=" + mode);
    String out;
    if (readSerial("OK", out))
    {
        Serial.println(out);
        return true;
    }

    return false;
}

bool Simcom::getIpDevice()
{
    Serial2.println("at+ipddr");
    String out;
    if (readSerial("OK", out))
    {
        Serial.println(out);
        return true;
    }

    return false;
}

bool Simcom::connectTCP(String ip, String port)
{
    String command = "at+cipopen=0,";
    command += "\"TCP\",\"";
    command += "\"" + ip + "\",";
    command += port;

    Serial2.println(command);
    String out;
    if (readSerial("OK", out))
    {
        Serial.println(out);
        return true;
    }

    return false;
}

bool Simcom::connectMQTTSecure(MQTTStruct req)
{
    String out;

    int MQTTProtocolNameLength = strlen(req.protocolName);
    int MQTTClientIDLength = strlen(req.clientID);
    int MQTTUsernameLength = strlen(req.username);
    int MQTTPasswordLength = strlen(req.password);
    int datalength = MQTTProtocolNameLength + 2 + 4 + MQTTClientIDLength + 2 + MQTTUsernameLength + 2 + MQTTPasswordLength + 2;

    Serial2.println();
    Serial2.println("AT+CIPSEND=0,100");
    if (!readSerial("+CIPOPEN: 0,0", out))
        return false;
    if (out.indexOf("OK") > -1)
        return false;

    Serial2.write(0x10);

    int X = datalength;
    do
    {
        unsigned char encodedByte = X % 128;
        X = X / 128;
        if (X > 0)
        {
            encodedByte |= 128;
        }
        Serial2.write(encodedByte);
    } while (X > 0);
    Serial2.write(MQTTProtocolNameLength >> 8);
    Serial2.write(MQTTProtocolNameLength & 0xFF);
    Serial2.print(req.protocolName);
    Serial2.write(req.lvl);   // LVL
    Serial2.write(req.flags); // Flags

    Serial2.write(req.keepAlive >> 8);
    Serial2.write(req.keepAlive & 0xFF);

    Serial2.write(MQTTClientIDLength >> 8);
    Serial2.write(MQTTClientIDLength & 0xFF);
    Serial2.print(req.clientID);

    Serial2.write(MQTTUsernameLength >> 8);
    Serial2.write(MQTTUsernameLength & 0xFF);
    Serial2.print(req.username);

    Serial2.write(MQTTPasswordLength >> 8);
    Serial2.write(MQTTPasswordLength & 0xFF);
    Serial2.print(req.password);

    Serial2.write(0x1A);

    return true;
}

bool Simcom::sendSubscribePackage(MQTTStruct req, String topic)
{
    int topiclength = topic.length();
    int datalength = 2 + 2 + topiclength + 1;

    Serial2.println();
    Serial2.println("AT+CIPSEND==0,100");
    delay(3000);
    //memset(str, 0, 250);

    delay(1000);
    Serial2.write(0x82);

    int X = datalength;
    do
    {
        unsigned char encodedByte = X % 128;
        X = X / 128;
        if (X > 0)
        {
            encodedByte |= 128;
        }
        Serial2.write(encodedByte);
    } while (X > 0);
    Serial2.write(req.packageID >> 8);
    Serial2.write(req.packageID & 0xFF);
    Serial2.write(topiclength >> 8);
    Serial2.write(topiclength & 0xFF);
    Serial2.print(topic);
    Serial2.write(req.qos);
    Serial2.write(0x1A);

    return true;
}

bool Simcom::connectMQTT(MQTTStruct req)
{
    // Developing
    return false;
}

bool Simcom::requestHTTP(HTTPReqStruct req, HTTPReqHeader header[], int size, String &out)
{
    // Request AT command
    Serial2.println("AT+CHTTPACT=\"" + req.host + "\"," + req.port);
    if (!readSerial("+CHTTPACT: REQUEST", out))
        return endRequestHTTP();

    Serial2.println(req.method + " " + req.url + " HTTP/1.1");
    Serial2.println("Host: " + req.host);
    Serial2.println("User-Agent: ESP32Client");
    for (int i = 0; i < size; i++)
        Serial2.println(header[i].header + ": " + header[i].value);
    Serial2.println("Content-Length: " + String(req.body.length()));
    Serial2.println();
    Serial2.println(req.body);
    Serial2.write((char)26);
    Serial2.println();

    if (!readSerial("+CHTTPACT:", out))
        return endRequestHTTP();
    if (out.indexOf("\r\nOK\r\n") == -1)
        return endRequestHTTP();

    if (!readSerial("+CHTTPACT: 0", out))
        return endRequestHTTP();

    String temp;
    for (int index = out.indexOf("\r\n"); index != -1; out.remove(0, index + 2))
    {
        index = out.indexOf("\r\n");
        if (out.length() > 2)
            temp = out;
    }

    out = temp;

    return true;
}

bool Simcom::endRequestHTTP()
{
    delay(500);
    Serial2.write((char)26);
    Serial2.println();
    return false;
}

bool Simcom::readSerial(String wakeword, String &out)
{
    long timestamp = millis();
    out = "";
    while (millis() - timestamp < _interval)
    {
        if (Serial2.available() > 0)
        {
            String temp = Serial2.readStringUntil('\n') + "\n";
            if (temp.indexOf(wakeword) != -1)
                return true;
            else
                out += temp;
        }
    }

    return false;
}