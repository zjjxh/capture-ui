#include "serialport.h"
#include <QModbusRtuSerialMaster>
#include <QDebug>
#include <qelapsedtimer.h>
#include <QCoreApplication.h>
#include <QtMath>

MySerialPort::MySerialPort(QObject *parent) : QObject(parent){
    mySerialPortInfoList = availablePorts();
    lastRequest= nullptr;
    modbusDevice = nullptr;
    speed[0] = 1;
    speed[1] = 1;
    speed[2] = 1;
    qDebug() << "run  myserialport class constructor" ;
}
QList<QSerialPortInfo> MySerialPort::availablePorts(void){
    QList<QSerialPortInfo>    list;
    foreach(QSerialPortInfo portInfo, QSerialPortInfo::availablePorts()) {
        if (!isSystemPort(&portInfo)) {
            list << *((QSerialPortInfo*)&portInfo);
            qDebug() << "portName    :" << portInfo.portName();    //调试时可以看的串口信息
            qDebug() << "Description :" << portInfo.description();
            qDebug() << "Manufacturer:" << portInfo.manufacturer();
            myPortName.append(portInfo.portName());
        }
    }
    emit portNameChanged(myPortName);      //通知前端comNameCombox下拉菜单更新
    return list;
}
bool MySerialPort::isSystemPort(QSerialPortInfo* port){
    // Known operating system peripherals that are NEVER a peripheral
    // that we should connect to.
    // XXX Add Linux (LTE modems, etc) and Windows as needed
    // MAC OS
    if (port->systemLocation().contains("tty.MALS")
        || port->systemLocation().contains("tty.SOC")
        || port->systemLocation().contains("tty.Bluetooth-Incoming-Port")
        // We open these by their cu.usbserial and cu.usbmodem handles
        // already. We don't want to open them twice and conflict
        // with ourselves.
        || port->systemLocation().contains("tty.usbserial")
        || port->systemLocation().contains("tty.usbmodem")) {
        return true;
    }
    return false;
}

void MySerialPort::serialPortOperate(int PortNameIndex, int BaudRateIndex, int StopbitsIndex, int DatabitsIndex, int ParityIndex){
    qDebug() << "run  myserialport operator" ;
    if(myLinkStatus ==false) // 目前尚无连接
    {
        qDebug()<<"portNameIndex:"<<QString::number(PortNameIndex);
        qDebug()<<"baudNameIndex:"<<QString::number(BaudRateIndex);
        qDebug()<<"stopNameIndex:"<<QString::number(StopbitsIndex);
        qDebug()<<"dataNameIndex:"<<QString::number(DatabitsIndex);
        qDebug()<<"parityNameIndex:"<<QString::number(ParityIndex);
        modbusDevice = new QModbusRtuSerialMaster(this);
        modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter,myPortName[PortNameIndex]);
        modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter,0);
        modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,9600);
        modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,8);
        modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,1);
        modbusDevice->setTimeout(200);//200ms
        modbusDevice->setNumberOfRetries(3);
        if (modbusDevice->state() == QModbusDevice::ConnectedState)
        {
            qDebug("COM already open");
            return;
        }
        /*
        mySerialPort = new QSerialPort();        //串口类对象的实例化
        if (mySerialPort->isOpen()){
           qDebug("COM already open");
           return;
        }
        mySerialPort->setPortName(myPortName[PortNameIndex]);
        mySerialPort->setBaudRate(115200);
        mySerialPort->setDataBits(QSerialPort::Data8);
        mySerialPort->setParity(QSerialPort::NoParity);
        mySerialPort->setFlowControl( QSerialPort::NoFlowControl );
        mySerialPort->setStopBits(QSerialPort::OneStop);
        myLinkStatus = mySerialPort->open(QIODevice::ReadWrite);
        if(myLinkStatus){
            mySerialPort->setDataTerminalReady(true);
            qDebug() << mySerialPort->portName() + " is open";
        }else {
            qDebug("Uart not exist or being occupied");
            return;
        }*/
        myLinkStatus = modbusDevice->connectDevice();
        if(myLinkStatus)
        {
            qDebug() << myPortName[PortNameIndex] + " is open";
            read_default_val();
        }
        else {
            qDebug("Uart not exist or being occupied");
            return;
        }
        emit linkStatusChanged(myLinkStatus);//通知前端连接状态变化
 //串口类对象的实例化，后连接readyRead信号 到本类的 readMyCom槽
 //connect(mySerialPort,SIGNAL(readyRead()),this,SLOT(readMyCom()),Qt::QueuedConnection);
//connect(mySerialPort, &MySerialPort::bytesReceived, _mavlinkProtocol,  &MAVLinkProtocol::receiveBytes);
    } else  //存在连接 要断开
    {
        if(modbusDevice->state() == QModbusDevice::ConnectedState)
         {
             qDebug() << myPortName[PortNameIndex] + " is close";
             modbusDevice->disconnectDevice();
             myLinkStatus=false;
             emit linkStatusChanged(myLinkStatus);//通知前端连接状态变化
         }
    }
}

void MySerialPort::read_default_Ready()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit readunit = reply->result();
        for (uint i = 0; i < readunit.valueCount(); i++) {
            //const QString entry = tr("Address: %1, Value: %2").arg(unit.startAddress() + i)
            //                         .arg(QString::number(unit.value(i),
            //                              unit.registerType() <= QModbusDataUnit::Coils ? 10 : 16));
            //reg |= readunit.value(i) << (16*i);
            def_val = readunit.values();
        }
        //qDebug("val of reg is:%x", reg);
    } else if (reply->error() == QModbusDevice::ProtocolError) {
        qDebug() << reply->errorString();
    } else {
        qDebug() << reply->errorString();
    }
    reply->deleteLater();
}

void MySerialPort::modify_speed(uint8_t num,uint8_t val,uint16_t def_speed)
{
    if(speed[num] == val)
        return;
    QModbusDataUnit writeUnit(QModbusDataUnit::HoldingRegisters,8,1);
    writeUnit.setStartAddress(10*num+5);

    writeUnit.setValue(0, def_speed*qPow(2, val));
    speed[num] = val;
    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, 1)) {
        if (!reply->isFinished())
         {
             connect(reply, &QModbusReply::finished, this, [this, reply]()
             {
                 if (reply->error() != QModbusDevice::NoError)
                     qDebug("modify_speed failed");
                 reply->deleteLater();
                 });
         }
         else
         {
             // broadcast replies return immediately
             reply->deleteLater();
         }
    } else {
        qDebug() << "Write error: "+modbusDevice->errorString();
    }
    QElapsedTimer t;
    t.start();
    while(t.elapsed()<1000)
        QCoreApplication::processEvents();
}

void MySerialPort::read_default_val()
{
    QModbusDataUnit readUnit(QModbusDataUnit::HoldingRegisters, 0, 30);

    if (auto *reply = modbusDevice->sendReadRequest(readUnit, 1)) {
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, &MySerialPort::read_default_Ready);
        else
            delete reply; // broadcast replies return immediately
    } else {
        qDebug() << "read_defult_val failed";
    }
}

void MySerialPort::motor_real_run(uint8_t num, int offset)
{
    QModbusDataUnit writeUnit(QModbusDataUnit::Coils,0,1);
    writeUnit.setStartAddress(num*5+offset);
    writeUnit.setValue(0, 1);
    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, 1)) {
        if (!reply->isFinished())
         {
             connect(reply, &QModbusReply::finished, this, [this, reply]()
             {
                 if (reply->error() != QModbusDevice::NoError)
                     qDebug("sendWriteRequest failed");
                 reply->deleteLater();
                 });
         }
         else
         {
             // broadcast replies return immediately
             reply->deleteLater();
         }
    } else {
        qDebug() << "Write error: "+modbusDevice->errorString();
    }
    QElapsedTimer t;
    t.start();
    while(t.elapsed()<1000)
        QCoreApplication::processEvents();
}

void MySerialPort::motor_move(uint8_t num, int angel, int dir, uint8_t speed_val)
{
    int motor_len;
    uint16_t def_speed;
    if(rw_wait == 1)
    {
        qDebug() << "last motor_move is not finish";
        return;
    }
    rw_wait = 1;
    motor_len = def_val[num*10+6] | (def_val[num*10+7] << 16);
    def_speed = def_val[num*10+5];
    modify_speed(num, speed_val, def_speed);

    QModbusDataUnit writeUnit(QModbusDataUnit::HoldingRegisters,8,2);
    writeUnit.setStartAddress(num*10+8);
    int val = (angel*motor_len)/360;
    writeUnit.setValue(0, val&0xFFFF);
    writeUnit.setValue(1, val>>16);
    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, 1)) {
        if (!reply->isFinished())
         {
             connect(reply, &QModbusReply::finished, this, [this, reply]()
             {
                 if (reply->error() != QModbusDevice::NoError)
                     qDebug("sendWriteRequest failed");
                 reply->deleteLater();
                 });
         }
         else
         {
             // broadcast replies return immediately
             reply->deleteLater();
         }
    } else {
        qDebug() << "Write error: "+modbusDevice->errorString();
    }
    QElapsedTimer t;
    t.start();
    while(t.elapsed()<1000)
        QCoreApplication::processEvents();
    QModbusDataUnit writeUnit2(QModbusDataUnit::HoldingRegisters,4,1);
    writeUnit.setStartAddress(num*10+4);
    writeUnit2.setValue(0, dir);
    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit2, 1)) {
        if (!reply->isFinished())
         {
             connect(reply, &QModbusReply::finished, this, [this, reply]()
             {
                 if (reply->error() != QModbusDevice::NoError)
                     qDebug("sendWriteRequest2 failed");
                 reply->deleteLater();
                 });
         }
         else
         {
             // broadcast replies return immediately
             reply->deleteLater();
         }
    } else {
        qDebug() << "Write error: "+modbusDevice->errorString();
    }
    motor_real_run(num, 0);
    rw_wait = 0;
}

/*run func run to end or push stop*/
void MySerialPort::motor_run(uint8_t num, int dir, uint8_t speed_val)
{
    uint16_t def_speed;
    if(rw_wait == 1)
    {
        qDebug() << "last motor_move is not finish";
        return;
    }
    rw_wait = 1;
    def_speed = def_val[num*10+5];
    modify_speed(num, speed_val, def_speed);

    if(dir == 0)
        motor_real_run(num, 1);
    if(dir == 1)
        motor_real_run(num, 2);
    if(dir == 2)
        motor_real_run(num, 4);
    rw_wait = 0;
}

void MySerialPort::readMyCom()
{
    qint64 byteCount = mySerialPort->bytesAvailable();
    if (byteCount) {
      QByteArray buffer;
      buffer.resize(byteCount);
      mySerialPort->read(buffer.data(), buffer.size());
      emit bytesReceived(buffer);  //通知解析类
   }
   else {
        // Error occurred
        qWarning() << "Serial port not readable";
   }
}
