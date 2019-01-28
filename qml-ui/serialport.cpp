#include "serialport.h"
#include <QDebug>
MySerialPort::MySerialPort(QObject *parent) : QObject(parent){
    mySerialPortInfoList = availablePorts();
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
    if(myLinkStatus==false) // 目前尚无连接
    {
        qDebug()<<"portNameIndex:"<<QString::number(PortNameIndex);
        qDebug()<<"baudNameIndex:"<<QString::number(BaudRateIndex);
        qDebug()<<"stopNameIndex:"<<QString::number(StopbitsIndex);
        qDebug()<<"dataNameIndex:"<<QString::number(DatabitsIndex);
        qDebug()<<"parityNameIndex:"<<QString::number(ParityIndex);
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
        }
        emit linkStatusChanged(myLinkStatus);//通知前端连接状态变化
 //串口类对象的实例化，后连接readyRead信号 到本类的 readMyCom槽
 connect(mySerialPort,SIGNAL(readyRead()),this,SLOT(readMyCom()),Qt::QueuedConnection);
//connect(mySerialPort, &MySerialPort::bytesReceived, _mavlinkProtocol,  &MAVLinkProtocol::receiveBytes);
    } else  //存在连接 要断开
    {
        if((mySerialPort->isOpen()))
         {
             qDebug() << mySerialPort->portName() + " is close";
             mySerialPort->clear();
             mySerialPort->close();
             mySerialPort->deleteLater();
             myLinkStatus=false;
             emit linkStatusChanged(myLinkStatus);//通知前端连接状态变化
         }
    }
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
