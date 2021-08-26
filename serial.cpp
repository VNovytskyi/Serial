#include "serial.h"

Serial::Serial()
{
    qDebug() << "SerialPort::constructor called";
}

void Serial::run()
{
    qDebug() << "SerialPort::run begin";

    serial = new QSerialPort;

    sheller_t shell;
    sheller_init(&shell);
uint8_t receivedDataBuff[SHELLER_USEFULL_DATA_LENGTH] = {0};
    while(runEnabled) {
        if (serial->isOpen()) {
            if (!transmittQueue.isEmpty()) {
                QByteArray txData = transmittQueue.first();
                transmittQueue.pop_front();

                uint8_t wrapperedDataBuff[SHELLER_PACKAGE_LENGTH] = {0};
                sheller_wrap(&shell, (uint8_t*)txData.data(), txData.size(), wrapperedDataBuff);

                serial->write(QByteArray((char*)wrapperedDataBuff, SHELLER_PACKAGE_LENGTH));
                serial->waitForBytesWritten(1);
                qDebug() << "Send";
            }

            serial->waitForReadyRead(1);

            if (serial->bytesAvailable() > 0) {
                qDebug() <<serial->bytesAvailable();
                QByteArray receiveData = serial->readAll();
                qDebug() << receiveData.toHex('.');

                for (int i = 0; i < receiveData.size(); ++i) {
                    sheller_push(&shell, receiveData[i]);
                }
            }


            if (sheller_read(&shell, receivedDataBuff) == SHELLER_OK) {
                QByteArray arr((char*)receivedDataBuff, SHELLER_USEFULL_DATA_LENGTH);
                qDebug() << "Sheller find the message: " << arr.toHex('.');

                receiveQueue.push_back(arr);
            }
        }
        //QThread().currentThread()->msleep(1);
    }

    qDebug() << "SerialPort::run end";
}

bool Serial::connectTo(QString portName, QString portSpeed)
{
    qDebug() << "connectTo " << portName << " ," << portSpeed;
    serial->setPortName(portName);
    serial->setBaudRate(portSpeed.toInt());
    serial->setReadBufferSize(64);

    return serial->open(QIODevice::ReadWrite);
}

void Serial::disconnect()
{
    serial->clear();
    serial->close();
}

QByteArray Serial::read()
{
    QByteArray data;

    if (!receiveQueue.isEmpty()) {
        data = receiveQueue.first();
        receiveQueue.pop_front();
    }

    return data;
}

bool Serial::write(QByteArray &data)
{
    if (data.length() > 0 && data.length() <= SHELLER_USEFULL_DATA_LENGTH) {
        transmittQueue.push_back(data);
        return true;
    }

    return false;
}

bool Serial::isEmpty()
{
    return receiveQueue.isEmpty();
}

void Serial::quit()
{
    qDebug() << "SerialPort::quit called";
    runEnabled = false;
}
