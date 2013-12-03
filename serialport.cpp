#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>
#include <fcntl.h>
#include <string.h>
#include <termio.h>
#include "serialport.h"
#include "log.h"
#include "mutex.h"
#include "autolock.h"
#include "threadif.h"
#include "serialportlistener.h"

//=================================
// MACROs
#define INVALID_FD				(-1)
#define	SP_THREAD_STACK_SIZE	(4096)

using namespace std;

class SerialRxThread : public ThreadIf {
	protected:
		friend class SerialPort;
		SerialRxThread(SerialPort *sp) : ThreadIf("SerialRxThread"), mSerialPort(*sp) {}
		virtual void thread() {
			SerialPort *sp = &mSerialPort;
			fd_set rset;
			struct timeval tv;
			int rc;
			InfoLog( "SerialPort:"<<sp->mPath<<": rx_thread start");
			tcflush(sp->mFd,TCIOFLUSH);   
			while(1)
			{
				{
					AutoLock lock(sp->mLock);
					if(sp->mState == SerialPort::CLOSING)
					{
						sp->mState = SerialPort::CLOSED;
						break;
					}
				}
				// IO operations & notify listener
				FD_ZERO(&rset);
				FD_SET(sp->mFd, &rset);
				tv.tv_sec = 1;
				tv.tv_usec = 0;
				rc = select((sp->mFd + 1) , &rset, NULL, NULL, &tv);
				if(rc == 0)
					//			InfoLog( "SerialPort:"<<sp->mPath<<": read timeout");
					rc = rc;
				else if(FD_ISSET(sp->mFd, &rset))
				{
					if(sp->mListeners.size() > 0)
					{
						vector<SerialPortListener *>::iterator itr = sp->mListeners.begin();
						while(itr != sp->mListeners.end())
						{
							rc = (*itr)->onReadable(*sp);
							itr++;
						}
					}
				}
				else
				{
					InfoLog( "SerialPort:"<<sp->mPath<<": select() error close port");
					if(sp->mListeners.size() > 0)
					{
						vector<SerialPortListener *>::iterator itr = sp->mListeners.begin();
						while(itr != sp->mListeners.end())
						{
							rc = (*itr)->onExcept(*sp);
							itr++;
						}
					}

				}
			}
			InfoLog(  "SerialPort:"<<sp->mPath<<": rx_thread exit");
		}
		SerialPort& mSerialPort;
};

class SerialTxThread : public ThreadIf {
	protected:
		friend class SerialPort;
		SerialTxThread(SerialPort *sp) : ThreadIf("SerialTxThread"), mSerialPort(*sp) {}
		virtual void thread() {
		}
		SerialPort& mSerialPort;
};


/**
 * @file SerialPort.cpp
 * @author TommyWang Email<xuchu1986@gmail.com>
 * @brief 
 */
static int _serial_baudrate_to_flag(int speed)
{
	static int baudflag_arr[] = {
		B921600, B460800, B230400, B115200, B57600, B38400,
		B19200, B9600, B4800, B2400, B1800, B1200,
		B600, B300, B150, B110, B75, B50
	};
	static int speed_arr[] = {
		921600, 460800, 230400, 115200, 57600, 38400,
		19200, 9600, 4800, 2400, 1800, 1200,
		600, 300, 150, 110, 75, 50
	};
	unsigned int i;
	for (i = 0; i < sizeof(speed_arr)/sizeof(int); i++) {
		if (speed == speed_arr[i]) {
			return baudflag_arr[i];
		}
	}
	return B9600;
}

int SerialPortRxThread(void *param)
{
}

//=================================
// public function implementations
/**
 * @brief constructor
 */
SerialPort::SerialPort() :
	mBaudrate(9600),
	mDatabits(8),
	mStopbits(1),
	mParity('n'),
	mIOtype(O_RDWR),
	mState(CREATED),
	mFd(INVALID_FD),
	mLock(new Mutex())
{
	memset(mPath, 0, sizeof(mPath));
	strcpy(mPath, "/dev/ttyS0");
}

/**
 * @brief destructor
 */
SerialPort::~SerialPort()
{
	close();
}

int SerialPort::setup(int baud, int databits, int stopbits, int parity)
{
	static struct termio oterm_attr;
	struct termio term_attr;
	/* Get current setting */
	if (ioctl(mFd, TCGETA, &term_attr) < 0) {
		return -1;
	}
	/* Backup old setting */
	memcpy(&oterm_attr, &term_attr, sizeof(struct termio));
	term_attr.c_iflag &= ~(INLCR | IGNCR | ICRNL | ISTRIP | IXON | BRKINT | INPCK);
	term_attr.c_oflag &= ~(OPOST | ONLCR | OCRNL);
	term_attr.c_lflag &= ~(ISIG | ECHO | ICANON | NOFLSH);
	term_attr.c_cflag &= ~CBAUD;
	term_attr.c_cflag |= CLOCAL | CREAD | _serial_baudrate_to_flag(baud);
	/* Set databits */
	term_attr.c_cflag &= ~(CSIZE);
	switch (databits) {
		case 5:
			term_attr.c_cflag |= CS5;
			break;
		case 6:
			term_attr.c_cflag |= CS6;
			break;
		case 7:
			term_attr.c_cflag |= CS7;
			break;
		case 8:
		default:
			term_attr.c_cflag |= CS8;
			break;
	}
	/* Set parity */
	switch (parity) {
		case 1: /* Odd parity */
			term_attr.c_cflag |= (PARENB | PARODD);
			break;
		case 2: /* Even parity */
			term_attr.c_cflag |= PARENB;
			term_attr.c_cflag &= ~(PARODD);
			break;
		case 0: /* None parity */
		default:
			term_attr.c_cflag &= ~(PARENB);
			break;
	}
	/* Set stopbits */
	switch (stopbits) {
		case 2: /* 2 stopbits */
			term_attr.c_cflag |= CSTOPB;
			break;

		case 1: /* 1 stopbits */
		default:
			term_attr.c_cflag &= ~CSTOPB;
			break;
	}
	term_attr.c_cc[VMIN] = 1;
	term_attr.c_cc[VTIME] = 0;
	if (ioctl(mFd, TCSETAW, &term_attr) < 0) {
		return -1;
	}
	if (ioctl(mFd, TCFLSH, 2) < 0) {
		return -1;
	}
	return 0;
}

int SerialPort::open(const char *file, int baudrate, int databits, int stopbits, char parity)
{
	/* check if port is already opened */
	AutoLock lock(mLock);
	int rc;
	if(mState == OPENED || mState == CLOSING)
		return 0;

	/* open serial port */
	mFd = ::open(file, mIOtype, 0);
	if(mFd < 0)
	{
		mFd = INVALID_FD;
		InfoLog(  "SerialPort:"<<file<<": open failed");
		return mFd;
	}
	/* execute m_setup */
	rc = setup(baudrate, 
			databits, 
			stopbits, 
			parity);
	if(rc < 0)
	{
		::close(mFd);
		mFd = INVALID_FD;
		InfoLog(  "SerialPort:"<<file<<": setup failed ");
		return rc;
	}
	/* create thread for receive bytes */
	mRxThread = new SerialRxThread(this);
	rc = mRxThread->Start();
	if(rc != 0)
	{

		::close(mFd);
		mFd = INVALID_FD;
		delete mRxThread;
		mRxThread = NULL;
		InfoLog(  "SerialPort:"<<file<<": fail to create rx thread");
		return rc;
	}
	/* update status & m_setup infomation */
	mState = OPENED;	
	mBaudrate = baudrate;
	mDatabits = databits;
	mStopbits = stopbits;
	mParity   = parity;
	strcpy(mPath, file);
	InfoLog(   "SerialPort:"<<file<<": open successfully with configuration "<< \
			mBaudrate<<','<< \
			mDatabits<<','<< \
			mStopbits<<','<< \
			mParity);
	return 0;
}

int SerialPort::reopen()
{
	return open(mPath, mBaudrate, mDatabits, mStopbits, mParity);
}

bool SerialPort::isOpen()
{
	return (mState == OPENED);
}

int SerialPort::baudrate()
{
	return mBaudrate;
}

int SerialPort::databits()
{
	return mDatabits;
}

int SerialPort::stopbits()
{
	return mStopbits;
}

char SerialPort::parity()
{
	return mParity;
}

int SerialPort::close()
{
	int rc;
	{
		AutoLock lock(mLock);
		if(mState == CLOSED || mState == CLOSING)
		{
			return 0;
		}
		rc = ::close(mFd);
		if(rc != 0)
			return rc;
		mFd = INVALID_FD;
		mState = CLOSING;
	}
	InfoLog(  "SerialPort:"<<mPath<<": closing");
	mRxThread->Join();
	delete mRxThread;
	InfoLog(  "SerialPort:"<<mPath<<": closed");
	return 0;
}

int SerialPort::removeListener(SerialPortListener* listener)
{
	AutoLock lock(mLock);
	vector<SerialPortListener*>::iterator i = mListeners.begin();
	while(i!= mListeners.end() && *i != listener)
		++i;
	if(i!=mListeners.end())
		mListeners.erase(i);
	InfoLog(  "SerialPort:"<<mPath<<": clear listener "<<listener);
	return 0;
}

int SerialPort::addListener(SerialPortListener *listener)
{
	AutoLock lock(mLock);
	mListeners.push_back(listener);
	InfoLog(  "SerialPort:"<<mPath<<": set listener "<<listener);
	return 0;
}

int SerialPort::read(void *buf, int len)
{
	if(mState != OPENED)
		return -1;
	return ::read(mFd, buf, len);
}

int SerialPort::nRead(void *buf, int len)
{
	if(mState != OPENED)
		return -1;
	int total = len;
	int n_read = 0;
	int left = len;
	char *buff = (char *)buf;
	while(left > 0)
	{
		n_read += read(buff+n_read, left);
		left = total - n_read;
	}
	return total;
}

int SerialPort::write(void *buf, int len)
{
	if(mState != OPENED)
		return -1;
	return ::write(mFd, buf, len);
}

void SerialPort::dump()
{
	const char status_str[3][20] = {
		{"closed"},
		{"open"},
		{"closing"}
	};
	InfoLog(  "device   :\t"<<mPath);
	InfoLog(  "baudrate :\t"<<mBaudrate);
	InfoLog(  "databits :\t"<<mDatabits);
	InfoLog(  "stopbits :\t"<<mStopbits);
	InfoLog(  "parity   :\t"<<mParity);
	InfoLog(  "status   :\t"<<status_str[mState]);
}

