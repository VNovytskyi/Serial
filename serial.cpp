#include "serial.h"

Serial::Serial()
{
    serial = new QSerialPort;
}

Serial::~Serial()
{
    delete serial;
}

void Serial::run()
{
    shell = new sheller_t;
    sheller_init(shell, shellerStartByte, shellerDataLength, shellerReceiveBuffSize);
    receivedMessage = new uint8_t(shellerDataLength);
    wrapperedDataBuff = new uint8_t(sheller_get_package_length(shell));

    while(runEnabled) {
        if (serial->isOpen()) {
            if (!transmittQueue.isEmpty()) {
                auto txData = transmittQueue.first();
                transmittQueue.pop_front();

                sheller_wrap(shell, (uint8_t*)txData.data(), txData.size(), wrapperedDataBuff);

                serial->write(QByteArray((char*)wrapperedDataBuff, sheller_get_package_length(shell)));
                serial->waitForBytesWritten(1);
            }

            serial->waitForReadyRead(1);
            if (serial->bytesAvailable()) {
                auto receiveData = serial->readAll();

                for (auto i = 0; i < receiveData.size(); ++i) {
                    sheller_push(shell, receiveData[i]);
                }
            }

            if (sheller_read(shell, receivedMessage) == SHELLER_OK) {
                QByteArray arr((char*)receivedMessage, shellerDataLength);
                receiveQueue.push_back(arr);
            }
        } else {
            QThread().currentThread()->msleep(1);
        }
    }
}

bool Serial::setSheller(uint8_t startByte, uint8_t dataLength, uint16_t receiveBuffSize)
{
    this->shellerStartByte = startByte;
    this->shellerDataLength = dataLength;
    this->shellerReceiveBuffSize = receiveBuffSize;

    return true;
}

bool Serial::connectTo(QString portName, QString portSpeed)
{
    serial->setPortName(portName);
    serial->setBaudRate(portSpeed.toInt());
    serial->setReadBufferSize(64);
    return serial->open(QIODevice::ReadWrite);
}

void Serial::disconnect()
{
    serial->clear();
    serial->close();

    delete shell;
    delete receivedMessage;
    delete wrapperedDataBuff;
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
    if (data.length() > 0 && data.length() <= shellerDataLength) {
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
    runEnabled = false;
}
