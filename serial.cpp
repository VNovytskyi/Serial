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

void Serial::setName(const quint32 &index)
{
    setName("COM" + QString::number(index));
}

void Serial::setSpeed(const QString &speed)
{
    int speedValue = speed.toInt();
    if (speedValue != 0) {
        setSpeed(speedValue);
    } else {
        qDebug() << "[ Serial ][ ERROR ] Cannot apply port speed. Method .toInt() return 0";
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
    if (0 < startByte && startByte < 255) {
        shellerStartByte = startByte;
    } else {
        qDebug() << "[ Serial ][ ERROR ] Cannot set start byte for sheller";
    }
}

void Serial::setStartByte(const QString &startByte)
{
    setStartByte((uint8_t)startByte.toInt());
}

void Serial::setDataLength(const quint32 dataLength)
{

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
                QByteArray arr((char*)receivedMessage, shellerDataLength);
                receiveQueue.push_back(arr);
            }
        } else {
            QThread().currentThread()->msleep(1);
        }
    }
}

bool Serial::open()
{
    return serial.open(QIODevice::ReadWrite);
}

void Serial::close()
{
    if (serial.isOpen()) {
        serial.clear();
        serial.close();
    }

    sheller_deinit(shell);
    delete_obj(shell);
    delete_arr(receivedMessage);
    delete_arr(wrapperedDataBuff);
    //delete porter
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
    if (data.length() > 0 && data.length() <= shellerDataLength) {
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

bool Serial::setSheller(uint8_t startByte, uint8_t dataLength, uint16_t receiveBuffSize)
{
    this->shellerStartByte = startByte;
    this->shellerDataLength = dataLength;
    this->shellerReceiveBuffSize = receiveBuffSize;

    shell = new sheller_t;
    sheller_init(shell, shellerStartByte, shellerDataLength, shellerReceiveBuffSize);
    receivedMessage   = new uint8_t [shellerDataLength];
    wrapperedDataBuff = new uint8_t [sheller_get_package_length(shell)];

    return true;
}

bool Serial::setPorter(uint8_t dataLength)
{
    porter = new porter_t;
    porter_init(porter, dataLength, [](const uint8_t *data, const uint8_t data_length){
        //send
    }, [](const uint8_t *data, const uint8_t data_length){
        //recv
    }, 100);
}






