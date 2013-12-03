#include "serialkeyboard.h"
#include "keyboardfactory.h"
#include "keyboardmessagehelper.h"
#include "serialport.h"
#include "threadif.h"
#include "log.h"

class QueryThread : public ThreadIf {
	protected:
		friend class KeyboardJXJ;
		QueryThread(SerialPort &keyboard) : ThreadIf("QueryThread for JXJKeyboard"), mKeyboard(keyboard) {}
	protected:
		virtual void thread()
		{
			char query = 0x50;
			while(!isStop())	
			{
				usleep(300000);
				mKeyboard.write(&query, 1);
			}
		}
		SerialPort &mKeyboard;
};



class KeyboardJXJ : 
	public SerialKeyboard
{
	public:
		KeyboardJXJ(Postable *receiver, ModuleInfo *info) : SerialKeyboard(receiver, info) , mThread(NULL) {}
		virtual int start(void *param) {
			int res;
			if((res = SerialKeyboard::start(param)) < 0)
				return res;
			mThread = new QueryThread(mSerialPort);
			if(mThread == NULL)
				return -1;
			if((res = mThread->Start()) < 0)
			{
				stop();
				return -1;
			}
			return 0;
		}
		virtual void stop() {
			if(mThread)// && mThread->isRunning())
			{
				mThread->Stop();
				delete mThread;
				mThread = NULL;
			}
			SerialKeyboard::stop();
		}
		virtual int onReadable(SerialPort& sp)
		{
			static unsigned char confirm[] = {0xf2, 0x00, 0x01, 0x00, 0x40, 0x00, 0x00, 0x41, 0xff};
			char packet[10];
			// find start byte of packet
			do {
				sp.read(packet, 1);
			} while((0xff & packet[0]) != 0xff);
			sp.nRead(packet+1, 7);
			/*
			cout<<"rx:";
			for(int i=0;i<8;i++)
			{
				cout<<hex<<setfill('0')<<setw(2)<<(int)(0xff & packet[i])<<' ';
			}
			cout<<endl;
			cout<<dec;
			*/
			if(packet[2] == 0x41)
			{
				InfoLog("receive register");
				sp.write(confirm, sizeof(confirm));
				return 0;
			}
			Message *msg;
			if(packet[2] == 0x11)
				msg = KeyboardMessageHelper::buildSwitchMessage(handle(), -1, (int)packet[4], (int)packet[5]);
			else if(packet[2] = 0x21)
			{
				int zi_spd = 0, zo_spd = 0, vspd = 0, hspd = 0;
				hspd = packet[5] & 0x7f;
				vspd = packet[6] & 0x7f;
				if((packet[3] & 0xff) == 0x40)
					zi_spd = 1;
				else if((packet[3] & 0xff) == 0x80)
					zo_spd = 1;
				msg = KeyboardMessageHelper::buildPtzMessage(handle(), -1, (int)packet[4], zi_spd, zo_spd, vspd, hspd);
			}
			else
				return 0;
			mLastMessage = msg->clone();	
			mReceiver->post(msg);
			return 0;
		}
		virtual int onExcept(SerialPort& sp)
		{
			ErrLog("KeyboardJXJ's serialport has encounter a problem");
			return 0;
		}
	protected:
		QueryThread *mThread;
		Message *mLastMessage;
};

DECL_KEYBOARD_FACTORY(KeyboardJXJ);
