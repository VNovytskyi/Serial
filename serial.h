#ifndef SERIAL_H
#define SERIAL_H

#include <QQueue>
#include <QDebug>
#include <QObject>
#include <QThread>
#include <QByteArray>
#include <QSerialPort>
#include <QSerialPortInfo>

#define delete_obj(x) {delete x; x = nullptr;}
#define delete_arr(x) {delete[] x; x = nullptr;}

#define value_in_range(a, x, b) ((a <= x) && (x <= b))

extern "C" {
    #include "../porter/Source/porter.h"
    #include "../sheller/Source/sheller.h"
}

class Serial: public QObject
{
    Q_OBJECT

    bool loopEnabled = true;

    QSerialPort serial;
    QQueue<QByteArray> receiveQueue;
    QQueue<QByteArray> transmitQueue;

    uint8_t serialDataLength = 8;

    sheller_t *shell = nullptr;
    uint8_t shellerStartByte = 0x23;
    uint16_t shellerReceiveBuffSize = 128;
    uint8_t *receivedMessage = nullptr;
    uint8_t *wrapperedDataBuff = nullptr;

    porter_t *porter = nullptr;


public:
    explicit Serial(QObject *parent = nullptr);
    ~Serial();

    void setName (const QString &name);
    void setName (const quint32 &index);
    void setSpeed(const QString &speed);
    void setSpeed(const quint32 &speed);
    void setStartByte(const quint8 &startByte);
    void setStartByte(const QString &startByte);

    void setDataLength(const quint32 dataLength);

    void loop();

    bool open();
    void close();

    QByteArray read();
    bool write(const QByteArray &data);

    bool isOpen();
    bool isEmpty();

    bool setSheller(uint8_t startByte, uint8_t dataLength, uint16_t receiveBuffSize);
    bool setPorter(uint8_t dataLength);



signals:

public slots:
    void disableLoop();
};

#endif // SERIAL_H
