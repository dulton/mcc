#ifndef __SerialMatrix_h__
#define __SerialMatrix_h__
#include "matrix.h"
#include "matrixfactory.h"
#include "serialport.h"
#include "serialportlistener.h"
class SerialMatrix : 
	public Matrix,
	public SerialPortListener
{
	public:
		SerialMatrix(Postable *receiver, ModuleInfo *info) : Matrix(receiver, info) {}
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
#endif /* __SerialMatrix_h__ */

