#include "mccmatrixserial.h"
#include "asynctcpserver.h"
#include "configparser.h"
#include "serverprocess.h"
#include "log.h"
#include <iostream>
#include <signal.h>
#include <cstdlib>
#include <assert.h>
#include <sys/socket.h>
#include <stack>
#include <string.h>

using namespace std;

static bool finished = false;
#define OPT_DELAY 100000
enum Key {
	Key_Unknown = 0,
	Key_0 = 75,
	Key_1 = 62,  
	Key_2 = 63,
	Key_3 = 64,
	Key_4 = 66,
	Key_5 = 67,
	Key_6 = 68,
	Key_7 = 70,
	Key_8 = 71,
	Key_9 = 72,
	Key_Monitor = 61,
	Key_Camera = 73,
	Key_Enter = 76,
	Key_Up = 96,
	Key_Down = 97,
	Key_Left = 98,
	Key_Right = 99,
	Key_ZoomIn = 86,
	Key_ZoomOut = 87,
	Key_Far = 88,
	Key_Near = 89
};

class HoneyWellMatrix :
	public MCCMatrixSerial
{
	public:
		virtual int onReadable(SerialPort *sp){
			char buf[100];
			int len = sp->Read(buf, sizeof(buf));
			buf[len] = 0;
			InfoLog("Receive data from matrix len = "<<len);
			return 0;
		}
		virtual int onWritable(SerialPort *sp) { return 0;}
		virtual int onExcept(int error) { return 0;}
		void forward(void *buf, size_t len) {
			mSerialPort->Write(buf, len);
		}
		void forward2(void *buf, size_t len) {
			size_t i = 0;
			char *data = (char*)buf;
			for(i=0;i<len;i++, ++data)
			{
				mSerialPort->Write(data, 1);
				usleep(10000);
			}
		}

		void Switch(int keyboard_id, int camera_id, int monitor_id = -1)
		{
			if(monitor_id != -1)
				selectMonitor(keyboard_id, monitor_id);
			selectCamera(keyboard_id, camera_id);
		}

		void Ptz(int keyboard_id, Key key, int speed, int scaler)
		{
			if(!speed)
			{
				SendSingleKey(keyboard_id, Key_Up, 0);
				SendSingleKey(keyboard_id, Key_Down, 0);
				SendSingleKey(keyboard_id, Key_Left, 0);
				SendSingleKey(keyboard_id, Key_Right, 0);
				SendSingleKey(keyboard_id, Key_Near, 0);
				SendSingleKey(keyboard_id, Key_Far, 0);
				return;
			}
			int real_speed = speed * scaler;
			SendSingleKey(keyboard_id, key, real_speed);
		}
		void selectMonitor(int keyboard_id, int monitor_id)
		{
			static Key _map[10] = { Key_0,Key_1,Key_2,Key_3,Key_4,Key_5,Key_6,Key_7,Key_8,Key_9};
			std::stack<int> nums;
			int num;
			SendKey(keyboard_id, Key_Monitor);
			while(monitor_id != 0)
			{
				nums.push(monitor_id % 10);
				monitor_id /= 10;
			}
			while(!nums.empty())
			{
				num = nums.top();
				nums.pop();
				SendKey(keyboard_id, _map[num]);
			}
			SendKey(keyboard_id, Key_Enter);
		}
		void selectCamera(int keyboard_id, int camera_id)
		{
			static Key _map[10] = { Key_0,Key_1,Key_2,Key_3,Key_4,Key_5,Key_6,Key_7,Key_8,Key_9};
			std::stack<int> nums;
			int num;
			SendKey(keyboard_id, Key_Camera);
			while(camera_id!= 0)
			{
				nums.push(camera_id % 10);
				camera_id /= 10;
			}
			while(!nums.empty())
			{
				num = nums.top();
				nums.pop();
				SendKey(keyboard_id, _map[num]);
			}
			SendKey(keyboard_id, Key_Enter);

		}
	protected:
		void SendKey(int keyboard_id, Key key)
		{
			SendSingleKey(keyboard_id, key, 1);
			SendSingleKey(keyboard_id, key, 0);
		}

		void SendSingleKey(int keyboard_id, Key key, int param)
		{
			char buf[20];
			sprintf(buf, "*8%02dK0,%d,%d\r", keyboard_id, key, param);
			InfoLog("Send key:"<<buf);
			forward2(buf, strlen(buf));
//			mSerialPort->Write(buf, strlen(buf));
			usleep(OPT_DELAY);
		}
};

class HoneyWellKeyboard :
	public MCCMatrixSerial
{
	public:
		HoneyWellKeyboard(HoneyWellMatrix *matrix) : mMatrix(matrix) { assert(matrix);}
		virtual int onReadable(SerialPort *sp){
			char buf[30];
			int len = sp->Read(buf, 1);
			buf[len] = 0;
			if(buf[0] != 0x7f)
			{
			//	InfoLog("Receive data from keyboard len = "<<len<<" "<<buf);
				mMatrix->forward(buf, len);
			}
			return 0;
		}
		virtual int onWritable(SerialPort *sp) { return 0;}
		virtual int onExcept(int error) { return 0;}
	protected:
		HoneyWellMatrix *mMatrix;
};


class HoneyWellForwardService : 
	public AsyncTCPServerHandler,
	public ServerProcess
{
	public:
		HoneyWellForwardService() : mConnected(false), mClientSock(-1), mMatrix(NULL), mServer(NULL), mState(CREATED), mConfig(NULL)
		{
		}

		enum State {
			CREATED,
			STARTED,
			STOPED
		};

		int start(int argc, char *argv[], const char *filename) 
		{
			if(!mConfig)
			{
				mConfig = new ConfigParser;
				assert(!mConfig->Parse(argc, argv, filename));
			}
			if(mConfig->getConfigBool("Misc","Daemonize", false))
				Daemonize();
			if(mState == STARTED)
				return 0;
			mMatrix = new HoneyWellMatrix;
			assert(mMatrix);
			const char *path = mConfig->getConfigString("Matrix", "Path", "/dev/ttyUSB0");
			const char *setup = mConfig->getConfigString("Matrix", "Setup", "19200,7,1,e");
			SerialPortParam param(path, setup);
//			SerialPortParam param("/dev/ttyUSB0",19200,7,1,'e');
			if(mMatrix->init(&param))
			{
				delete mMatrix;
				mMatrix = NULL;
				ErrLog("Failed to init matrix");
				return -1;
			}
			if(mConfig->getConfigBool("Keyboard","Enable",false))
			{
				mKeyboard = new HoneyWellKeyboard(mMatrix);
				path = mConfig->getConfigString("Keyboard", "Path", "/dev/ttyUSB1");
				setup = mConfig->getConfigString("Keyboard", "Setup", "19200,7,1,e");
				SerialPortParam param1(path, setup);
				if(mKeyboard->init(&param1))
				{
					mMatrix->deinit();
					delete mMatrix;
					mMatrix = NULL;
					delete mKeyboard;
					mKeyboard= NULL;
					ErrLog("Failed to init keyboard");
					return -1;
				}
			}
			mServer = new AsyncTCPServer(this);
			int port = mConfig->getConfigInt("Server", "Port", 10000);
			mKeyboardId = mConfig->getConfigInt("Misc", "KeyboardId", 1);
			mSpeedScaler = mConfig->getConfigInt("Misc", "SpeedScaler", 3);
			if(mServer->Start(NULL, port))
			{
				mMatrix->deinit();
				delete mMatrix;
				mMatrix = NULL;
				delete mServer;
				mServer = NULL;
				ErrLog("Failed to start tcp server");
				return -1;
			}
			mState = STARTED;
			return 0;
		}

		void stop() 
		{
			if(mState != STARTED)
				return;
			// deinit matrix
			mMatrix->deinit();
			delete mMatrix;
			mMatrix = NULL;

			// stop tcp server
			mServer->Stop();
			delete mServer;
			mServer = NULL;
			mState = STOPED;
		}
	protected:
		bool validpacket(void *buf, size_t len)
		{
			char *data = (char*)buf;
			if(data[0] != '*')
				return false;
			if(data[1] != '8')
				return false;
			if(data[2] < '0' || data[2] > '9')
				return false;
			if(data[3] < '0' || data[3] > '9')
				return false;
			if(data[4] != 'K')
				return false;
			if(data[5] != '0')
				return false;
			if(data[6] != '0')
				return false;
			if(data[7] != '0')
				return false;
		}
	


		/**
		 * called whenever there is a new client connected
		 * @param fd socket for client
		 */
		virtual void onNewConnection(int fd)
		{
			if(mConnected)
			{
//				InfoLog("Client already connected, refuse new connection");
//				close(fd);
				InfoLog("Client already connected, close previous");
				close(mClientSock);
				mClientSock = fd;
				// close fd
			}
			else
			{
				mConnected = true;
				mClientSock = fd;
			}
		}

		/**
		 * called whenever a socket is about to close don't
		 * close it in call back
		 * @param fd socket for client
		 */
		virtual void onDisconnect(int fd)
		{
			mConnected = false;
			mClientSock = -1;
		}

		/**
		 * called whenever a socket is readable
		 * @param fd socket for client
		 */
		virtual void onReadable(AsyncTCPServer &server, int fd, void *data, size_t len)
		{
			/* static variables used to store prev command */
			static int s_prev_camera_id = 0, s_prev_monitor_id = 0, s_prev_ptz_speed = 0;
			static Key s_prev_ptz_type = Key_Unknown;
			char *event = (char*)data;

			int camera_id, monitor_id, ptz_speed;
			Key ptz_type = Key_Up;
			char cptz_type;
			if(event[0] == '#')
			{
				//InfoLog("Receive data (Invalid): len = "<<len<<" "<<event);
				//InfoLog("Receive command: "<<event);
				char *packet = strstr(event, "#");
				while(packet)
				{
					char outbuf[20];
					char *p0 = outbuf, *p1 = packet;
					while(*p1 != '\r')
						*p0++ = *p1++;
					*p0 = 0;
					InfoLog("Parse command: "<<outbuf);
					if(packet[1] == 'S')
					{
						sscanf(packet, "#S%d,%d,%c,%d\r", &monitor_id, &camera_id, &cptz_type, &ptz_speed);
						if(camera_id != s_prev_camera_id || monitor_id != s_prev_monitor_id)
						{
							s_prev_camera_id = camera_id;
							s_prev_monitor_id = monitor_id;
							mMatrix->Switch(mKeyboardId, camera_id, monitor_id);
						}
					}
					else if(packet[1] == 'P')
					{
						sscanf(packet, "#P%d,%d,%c,%d\r", &monitor_id, &camera_id, &cptz_type, &ptz_speed);
						switch(cptz_type)
						{
							case 'U':
								ptz_type = Key_Up;
								break;
							case 'D':
								ptz_type = Key_Down;
								break;
							case 'R':
								ptz_type = Key_Right;
								break;
							case 'L':
								ptz_type = Key_Left;
								break;
							case 'N':
								ptz_type = Key_ZoomIn;
								break;
							case 'F':
								ptz_type = Key_ZoomOut;
								break;
							default:
								return;
						}
						if(s_prev_monitor_id != monitor_id)
						{
							mMatrix->selectMonitor(mKeyboardId, monitor_id);
							s_prev_monitor_id = monitor_id;
						}
						if(s_prev_monitor_id != monitor_id || s_prev_ptz_type != ptz_type || s_prev_ptz_speed != ptz_speed)
						{
							s_prev_ptz_type = ptz_type;
							s_prev_ptz_speed = ptz_speed;
							mMatrix->Ptz(mKeyboardId, ptz_type, ptz_speed, mSpeedScaler);
							//InfoLog("Parsed : monitor = "<<monitor_id<<" camera = "<<camera_id<<" ptz = "<<ptz_type<<" speed = "<<ptz_speed);
						}
					}
					packet = strstr(packet+1, "#");
				}
			}
//			else if(event[0] == '*')
//			{
//				InfoLog("Receive data : len = "<<len<<" "<<event);
//				mMatrix->forward2(event, len);
//			}
			else
			{
				InfoLog("Invalid command");
			}
		}

		/**
		 * called whenever a socket is writable
		 * @param fd socket for client
		 */
		virtual void onWriteble(AsyncTCPServer &server, int fd)
		{
			// do nothing
		}

		/**
		 * called whenever a socket has error 
		 * @param fd socket for client
		 */
		virtual void onExcept(int fd, int err)
		{
			mConnected = false;
			mClientSock = -1;
			ErrLog("fd "<<fd<<" error, code = "<<err);
		}
		bool			 mConnected;
		int				 mClientSock;
		HoneyWellMatrix	*mMatrix;
		HoneyWellKeyboard *mKeyboard;
		AsyncTCPServer	*mServer;
		State			 mState;
		ConfigParser	*mConfig;
		int				 mKeyboardId;
		int				 mSpeedScaler;
};

static void
signalHandler(int signo)
{
	static int cnt = 0;
	if(cnt == 0)
	{
		InfoLog("MCC system is going to shutdown ...");
		finished = true;
	}
	else
	{
		InfoLog("MCC system shutdown right now!");
		exit(0);
	}
	cnt++;
}

int main(int argc, char *argv[])
{
	// install signal handler
#ifndef _WIN32
	if ( signal( SIGPIPE, SIG_IGN) == SIG_ERR)
	{
		cerr << "Couldn't install signal handler for SIGPIPE" << endl;
		exit(-1);
	}
#endif

	if ( signal( SIGINT, signalHandler ) == SIG_ERR )
	{
		cerr << "Couldn't install signal handler for SIGINT" << endl;
		exit( -1 );
	}

	if ( signal( SIGTERM, signalHandler ) == SIG_ERR )
	{
		cerr << "Couldn't install signal handler for SIGTERM" << endl;
		exit( -1 );
	}
//	ConfigParser config(argc, argv, "mcc.config");
	HoneyWellForwardService service;
	assert(!service.start(argc, argv, "HoneyWell.ini"));
	while(!finished)
	{
		usleep(1000000);
	}
	service.stop();
	return 0;
}
