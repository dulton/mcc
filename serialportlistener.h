/**
 * @file serialportlistener.h
 * @author TommyWang Email<xuchu1986@gmail.com>
 * @brief 
 */
#ifndef __SERIALPORTLISTENER_H__
#define __SERIALPORTLISTENER_H__
class SerialPort;
class SerialPortListener {
	public:
		virtual ~SerialPortListener() {};
		virtual int onReadable(SerialPort& sp) = 0;
		virtual int onExcept(SerialPort& sp) = 0;
};


#endif /* __SERIALPORTLISTENER_H__ */
