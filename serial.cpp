#include "serial.h"

Serial::Serial(QObject *parent) : QObject(parent)
{
    qDebug() << "[ Serial ][ INFO ] 1";
}

Serial::~Serial()
{
    close();
}

void Serial::setName(const QString &name)
{
    if (name.contains("COM")) {
        serial.setPortName(name);
    } else {
        qDebug() << "[ Serial ][ ERROR ] Receive port name without 'COM' in title";
    }
}

void Serial::setSpeed(const quint32 &speed)
{
    if(!serial.setBaudRate(speed)) {
        qDebug() << "[ Serial ][ ERROR ] Cannot apply port speed";
    }
}

void Serial::setStartByte(const quint8 &startByte)
{
    if (value_in_range(SHELLER_STARTBYTE_MIN, startByte, SHELLER_STARTBYTE_MAX)) {
        shellerStartByte = startByte;
    } else {
        qDebug() << "[ Serial ][ ERROR ] Cannot set start byte for sheller";
    }
}

void Serial::setDataLength(const quint32 dataLength)
{
    if (value_in_range(1, dataLength, 254)) {
        serialDataLength = dataLength;
    } else {
        qDebug() << "[ Serial ][ ERROR ] Cannot set serialDataLength. Value not in a range";
    }
}

void Serial::setReceiveBufferSize(const quint32 size)
{
    if (value_in_range(1, size, 4096)) {
        shellerReceiveBuffSize = size;
    } else {
        qDebug() << "[ Serial ][ ERROR ] Cannot set shellerReceiveBuffSize. Value not in a range";
    }
}

void Serial::loop()
{
    while(loopEnabled) {
        if (serial.isOpen()) {

            if (!transmitQueue.isEmpty()) {
                auto txData = transmitQueue.first();
                transmitQueue.pop_front();

                sheller_wrap(shell, (uint8_t*)txData.data(), txData.size(), wrapperedDataBuff);
                serial.write(QByteArray((char*)wrapperedDataBuff, sheller_get_package_length(shell)));
                serial.waitForBytesWritten(1);
            }

            serial.waitForReadyRead(1);
            if (serial.bytesAvailable()) {
                auto receiveData = serial.readAll();

                for (auto i = 0; i < receiveData.size(); ++i) {
                    sheller_push(shell, receiveData[i]);
                }
            }

            if (sheller_read(shell, receivedMessage) == SHELLER_OK) {
                QByteArray arr((char*)receivedMessage, serialDataLength);
                receiveQueue.push_back(arr);
            }
        } else {
            QThread().currentThread()->msleep(1);
        }
    }
}

bool Serial::open()
{
    shell = new sheller_t;
    receivedMessage   = new uint8_t [serialDataLength];
    wrapperedDataBuff = new uint8_t [sheller_get_package_length(shell)];
    if(sheller_init(shell, shellerStartByte, serialDataLength, shellerReceiveBuffSize) == SHELLER_ERROR) {
        qDebug() << "[ Serial ][ ERROR ] Cannot init sheller";
        close();
    }

    porter = new porter_t;
    porter_init(porter, serialDataLength, [](const uint8_t *data, const uint8_t length){

    }, [](const uint8_t *data, const uint8_t length){

    }, 100);


    if (!serial.open(QIODevice::ReadWrite)) {
        close();
    }

    return true;
}

void Serial::close()
{
    if (serial.isOpen()) {
        serial.clear();
        serial.close();
    }

    sheller_deinit(shell);
    delete_obj(shell);
    delete_obj(porter);
    delete_arr(receivedMessage);
    delete_arr(wrapperedDataBuff);
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

bool Serial::write(const QByteArray &data)
{
    if (value_in_range(1, data.length(), serialDataLength)) {
        transmitQueue.push_back(data);
        return true;
    }

    return false;
}

bool Serial::isOpen()
{
    return serial.isOpen();
}

bool Serial::isEmpty()
{
    return receiveQueue.isEmpty();
}

void Serial::disableLoop()
{
    loopEnabled = false;
}
