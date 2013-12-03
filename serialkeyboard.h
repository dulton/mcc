#ifndef __SerialKeyboard_h__
#define __SerialKeyboard_h__
#include <string>
#include "keyboard.h"
#include "serialport.h"
#include "serialportlistener.h"

class SerialKeyboard :
	public Keyboard,
	public SerialPortListener 
{
	public:
		SerialKeyboard(HandleManager& mgr, Postable *receiver, ModuleInfo *info) : Keyboard(mgr, receiver, info) {}
		virtual int start(void *param) {
			const char *path = (const char*)param;
			long baudrate = mInfo->attribute("Baudrate", 9600l);
			long databits = mInfo->attribute("Databits", 8l);
			long stopbits = mInfo->attribute("Stopbits", 1l);
			std::string parity = mInfo->attribute("Parity", std::string("n"));
			return mSerialPort.open(path, baudrate, databits, stopbits, *(parity.c_str()));
		}
		virtual void stop() {
			mSerialPort.close();
		}
	protected:
		SerialPort mSerialPort;
};
#endif /* __SerialKeyboard_h__ */
