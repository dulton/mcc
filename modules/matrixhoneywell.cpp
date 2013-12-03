#include "serialmatrix.h"
#include "keyboardmessagehelper.h"
#include "log.h"
#include "honeywell.h"
#include <stack>
#include <string.h>

using namespace std;

class MatrixHoneyWell:
	public SerialMatrix
{
	public:
		MatrixHoneyWell(Postable *receiver, ModuleInfo *info) : SerialMatrix(receiver, info) {
			mKeyboardId = info->attribute("KeyboardId", 0l);
			mScaler = info->attribute("Scaler", 1.0);
		}
		virtual int onReadable(SerialPort& sp){
			char buf[100];
			int len = sp.read(buf, sizeof(buf));
			buf[len] = 0;
			InfoLog("Receive data from matrix len = "<<len);
			return 0;
		}
		virtual int onExcept(SerialPort& error) { return 0;}
		void forward(void *buf, size_t len) {
			mSerialPort.write(buf, len);
		}
		void forward2(void *buf, size_t len) {
			size_t i = 0;
			char *data = (char*)buf;
			for(i=0;i<len;i++, ++data)
			{
				mSerialPort.write(data, 1);
				usleep(10000);
			}
		}
		virtual void post(Message* msg)
		{
			KeyboardSwitchMessage *smsg = dynamic_cast<KeyboardSwitchMessage*>(msg);
			if(smsg)
			{
				Switch(mKeyboardId, smsg->camera(), smsg->monitor());
				DebugLog("honeywell matrix module receive a switch");
				return;
			}
			KeyboardPtzMessage *pmsg = dynamic_cast<KeyboardPtzMessage*>(msg);
			if(pmsg)
			{
				Key key;
				int speed;
				if(pmsg->vSpeed())
				{
					key = pmsg->vSpeed() > 0 ? Key_Up : Key_Down;
					speed = pmsg->vSpeed() > 0 ? pmsg->vSpeed() : -pmsg->vSpeed();
				}
				else if(pmsg->hSpeed())
				{
					key = pmsg->hSpeed() > 0 ? Key_Left : Key_Right;
					speed = pmsg->hSpeed() > 0 ? pmsg->hSpeed() : -pmsg->hSpeed();
				}
				else
				{
					key = pmsg->zoomInSpeed() ? Key_ZoomIn : Key_ZoomOut;
					speed = pmsg->zoomInSpeed() > 0 ? pmsg->zoomInSpeed() : pmsg->zoomOutSpeed();
				}
				DebugLog("honeywell matrix module receive a ptz");
				Ptz(mKeyboardId, key, speed, mScaler);
			}
		}

		void Switch(int keyboard_id, int camera_id, int monitor_id = -1)
		{
			if(monitor_id != -1)
				selectMonitor(keyboard_id, monitor_id);
			selectCamera(keyboard_id, camera_id);
		}

		void Ptz(int keyboard_id, Key key, int speed, double scaler)
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
			DebugLog("Send key:"<<buf);
			forward2(buf, strlen(buf));
			usleep(OPT_DELAY);
		}
	private:
		int mKeyboardId;
		double mScaler;
};

DECL_MATRIX_FACTORY(MatrixHoneyWell);
