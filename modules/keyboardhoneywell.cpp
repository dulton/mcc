#include "serialkeyboard.h"
#include "keyboardfactory.h"
#include "keyboardmessagehelper.h"
#include "log.h"
#include "honeywell.h"
#include <stack>
using namespace std;

// *8%02dK0,%d,%d\r
class KeyboardHoneyWell :
	public SerialKeyboard
{
	public:
		KeyboardHoneyWell(HandleManager& mgr, Postable *receiver, ModuleInfo *info) : SerialKeyboard(mgr, receiver, info) {
			// initiate keymap for fast transit
			for(int i = 0;i<sizeof(mKeyMap) / sizeof(int);++i)
				mKeyMap[i] = -1;
			mKeyMap[Key_0] = 0;
			mKeyMap[Key_1] = 1;
			mKeyMap[Key_2] = 2;
			mKeyMap[Key_3] = 3;
			mKeyMap[Key_4] = 4;
			mKeyMap[Key_5] = 5;
			mKeyMap[Key_6] = 6;
			mKeyMap[Key_7] = 7;
			mKeyMap[Key_8] = 8;
			mKeyMap[Key_9] = 9;
		}
		virtual const char* name() const { return "KeyboardHoneywell";}
		virtual int onReadable(SerialPort& sp){
			char buf[30];
			static char *p = buf;
			char c;
			static bool start;
			static int read_len;
			int len;
			len = sp.read(&c, 1);
			if(!start)
			{
				if(c == '*')
				{
					buf[0] = c;
					start = true;
					read_len = 1;
				}
			}
			else
			{
				buf[read_len] = c;
				read_len++;
				if(c == '\r')
				{
					int key, value;
					bool iskey = getKey(buf, read_len, key, value);
					read_len = 0;
					start = false;
					if(key != mLastKey				// press another key
							|| value == mLastValue  // same action on same key
							|| value != 0)			// action is not a key pop
					{
						mLastKey = key;
						mLastValue = value;
						return 0;
					}
					mLastKey = key;
					mLastValue = value;
					// FSM for keyboard honeywell
					Message *msg = operation_fsm_process(key, value);
					if(msg)
						mReceiver->post(msg);
				}
			}
			return 0;
		}
		virtual int onExcept(SerialPort& sp) { return 0;}
	protected:
		bool getKey(const char *buf, int len, int& key, int& value) { return false;}
		enum KeyInState {
			None,
			CameraNumberEntering,
			CameraNumberEntered,
			MonitorNumberEntered,
			MonitorNumberEntering
		};
		inline void clearqueue() { while(!mKeyIn.empty()) mKeyIn.pop();}
		inline int toNumber(int key) { return mKeyMap[key];}
		/*
		 *
		 */
		Message *operation_fsm_process(int key, int value)
		{
			switch(key)
			{
				case Key_Enter:
					{
						int num = 0;
						while(!mKeyIn.empty())
						{
							num *= 10;
							num += mKeyIn.front();
							mKeyIn.pop();
						}
						if(mState == CameraNumberEntering)
						{
							mSelectedCamera = num;
							mState = CameraNumberEntered;
							return KeyboardMessageHelper::buildSwitchMessage(handle(),
									-1,
									mSelectedCamera,
									mSelectedMonitor);
						}
						else if(mState == MonitorNumberEntering)
						{
							mSelectedMonitor = num;
							mState == MonitorNumberEntered;
						}
					}
					break;
				case Key_0:
				case Key_1:
				case Key_2:
				case Key_3:
				case Key_4:
				case Key_5:
				case Key_6:
				case Key_7:
				case Key_8:
				case Key_9:
					{
						int k = toNumber(key);
						assert(k != -1);
						mKeyIn.push(k);
						if(mState == None || mState == CameraNumberEntered)
							mState = CameraNumberEntering;
					}
					break;
				case Key_Camera:
					clearqueue();
					mState = CameraNumberEntering;
					break;
				case Key_Monitor:
					clearqueue();
					mState = MonitorNumberEntering;
					break;
				case Key_Up:
					return KeyboardMessageHelper::buildPtzMessage(handle(), 
							-1,
							mSelectedCamera,0,0,value,0);
					break;
				case Key_Down:
					return KeyboardMessageHelper::buildPtzMessage(handle(), 
							-1,
							mSelectedCamera,0,0,-value,0);
					break;
				case Key_Left:
					return KeyboardMessageHelper::buildPtzMessage(handle(), 
							-1,
							mSelectedCamera,0,0,0,value);
					break;
				case Key_Right:
					return KeyboardMessageHelper::buildPtzMessage(handle(), 
							-1,
							mSelectedCamera,0,0,0,-value);
					break;
				case Key_ZoomIn:
					return KeyboardMessageHelper::buildPtzMessage(handle(), 
							-1,
							mSelectedCamera,value,0,0,0);
					break;
				case Key_ZoomOut:
					return KeyboardMessageHelper::buildPtzMessage(handle(), 
							-1,
							mSelectedCamera,0,value,0,0);
					/*
					   msg = KeyboardMessageHelper::buildPtzMessage(handle(), 
					   -1,
					   mSelectedCamera,
					   (key == Key_ZoomIn ? value : 0),
					   (key == Key_ZoomOut ? value : 0),
					   (key == Key_Up ? value : (key == Key_Down ? -value : 0)),
					   (key == Key_Left ? value : (key == Key_Right ? -value : 0)));
					   */
					break;
				default:
					mState = None;
					clearqueue();
					break;
			}
			return NULL;
		}
		queue<int> mKeyIn;
		KeyInState mState;
		int mKeyMap[128];
		int mSelectedMonitor;
		int mSelectedCamera;
		int mLastKey;
		int mLastValue;
};

DECL_KEYBOARD_FACTORY(KeyboardHoneyWell);
