#ifndef SERIAL_H
#define SERIAL_H

#include <QQueue>
#include <QDebug>
#include <QObject>
#include <QThread>
#include <QByteArray>
#include <QSerialPort>
#include <QSerialPortInfo>

extern "C" {
    #include "../sheller/Source/sheller.h"
    #include "../porter/Source/porter.h"
}

class Serial: public QObject
{
    Q_OBJECT

    bool loopEnabled = true;

    QSerialPort serial;
    QQueue<QByteArray> receiveQueue;
    QQueue<QByteArray> transmitQueue;

    uint8_t shellerStartByte = 0x23;
    uint16_t shellerReceiveBuffSize = 128;
    uint8_t shellerDataLength = 8;
    uint8_t *receivedMessage = nullptr;
    uint8_t *wrapperedDataBuff = nullptr;
    sheller_t *shell = nullptr;

public:
    explicit Serial(QObject *parent = nullptr);
    ~Serial();

    void loop();

    bool setSheller(uint8_t startByte, uint8_t dataLength, uint16_t receiveBuffSize);
    bool connectTo(QString portName, QString portSpeed);
    void disconnect();

    QByteArray read();
    bool write(QByteArray &data);

    bool isConnected();
    bool isEmpty();

signals:

public slots:
    void disableLoop();
};

#endif // SERIAL_H
