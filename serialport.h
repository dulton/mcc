/**
 * @file SerialPort.h
 * @author TommyWang Email<xuchu1986@gmail.com>
 * @brief 
 */
#ifndef __SERIALPORT_H__
#define __SERIALPORT_H__
#include <vector>

class Mutex;
class ThreadIf;
class SerialPortListener;
class SerialRxThread;
class SerialTxThread;

class SerialPort {
	public:
		/**
		 * @brief constructor
		 */
		SerialPort();
		/**
		 * @brief destructor
		 */
		virtual ~SerialPort();
		int open(const char *file, int baudrate, int databits, int stopbits, char parity);
		int close();
		int read(void *buf, int len);
		int nRead(void *buf, int len);
		int write(void *buf, int len);
		int addListener(SerialPortListener *listener);
		int removeListener(SerialPortListener *listener);
		int reopen();
		bool isOpen();
		int baudrate();
		int databits();
		int stopbits();
		char parity();
		void dump();
	protected:
		friend class SerialRxThread;
		friend class SerialTxThread;
		enum State {
			CREATED,
			OPENED,
			CLOSING,
			CLOSED
		};
		int setup(int baudrate, int databits, int stopbits, int parity);
		int mBaudrate;
		int mDatabits;
		int mStopbits;
		char mParity;
		int mIOtype;
		char mPath[256];
		int mFd;
		State mState;
		Mutex *mLock;
		ThreadIf *mRxThread;
		ThreadIf *mTxThread;
		std::vector<SerialPortListener*> mListeners;
};

#endif /* __SERIALPORT_H__ */
