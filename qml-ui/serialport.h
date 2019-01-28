#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QObject>

class MySerialPort : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool linkStatus  READ linkStatus NOTIFY linkStatusChanged)
    Q_PROPERTY(QStringList portName  READ portName NOTIFY portNameChanged)
    Q_PROPERTY(QString strDisplay  READ strDisplay NOTIFY strDisplayChanged)//这里没有用到
public:
    explicit MySerialPort(QObject *parent = nullptr);
    QList<QSerialPortInfo> availablePorts();
    static bool isSystemPort(QSerialPortInfo *port);
    QStringList portName()const{
        return myPortName;
    }
    bool linkStatus()const{
        return myLinkStatus;
    }
    QString strDisplay()const{
        return hexStrDisplay;
    }
    Q_INVOKABLE void serialPortOperate(int PortNameIndex,int BaudRateIndex,int StopbitsIndex,int DatabitsIndex,int ParityIndex);
private:
    QSerialPort *mySerialPort;
    bool myLinkStatus=false;     //true connected  false unconnected
    QList<QSerialPortInfo> mySerialPortInfoList;
    QStringList myPortName;
    QString hexStrDisplay;
signals:
    void portNameChanged(QStringList);
    void linkStatusChanged(bool);
    void strDisplayChanged(QString);
    void bytesReceived(QByteArray data);
public slots:
    void readMyCom();
};
#endif // SERIALPORT_H
