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

public:
    Serial();

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
