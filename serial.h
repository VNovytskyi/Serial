#ifndef SERIAL_H
#define SERIAL_H

#include <QDebug>
#include <QThread>
#include <QObject>
#include <QByteArray>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QQueue>

#include "Sheller/Source/sheller.h"

class Serial: public QThread
{
    Q_OBJECT

    bool runEnabled = true;

    QSerialPort *serial = nullptr;
    QQueue<QByteArray> receiveQueue;
    QQueue<QByteArray> transmittQueue;

    uint8_t shellerStartByte = 0x23;
    uint16_t shellerReceiveBuff = 128;
    uint8_t shellerDataLength = 8;
    uint8_t *receivedMessage = nullptr;
    uint8_t *wrapperedDataBuff = nullptr;
    sheller_t *shell = nullptr;

public:
    Serial();
    ~Serial();

    void run() override;
    bool connectTo(QString portName, QString portSpeed);
    void disconnect();

    QByteArray read();
    bool write(QByteArray &data);
    bool isEmpty();

public slots:
    void quit();
};

#endif // SERIAL_H
