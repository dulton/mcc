#include "serialkeyboard.h"
#include "keyboardfactory.h"
#include "moduleinfo.h"
#include <iostream>
using namespace std;
class TestKeyboard : public SerialKeyboard {
	public:
		TestKeyboard(Postable *receiver, ModuleInfo* info) : SerialKeyboard(receiver, info) {
			cout<<"TestKeyboard created"<<endl;
		}
		virtual void post(Message* msg) {
			SerialKeyboard::post(msg);
			cout<<"TestKeyboard Module receive message"<<endl;
		}
		virtual int onReadable(SerialPort& sp) {
			return 0;
		}
		virtual int onExcept(SerialPort& sp) {
			return 0;
		}
};

class TestKeyboardFactory : public KeyboardFactory {
	public:
		virtual Keyboard* create(Postable *receiver) {
			if(mInfo)
				return new TestKeyboard(receiver, mInfo);
			return NULL;
		}
};

TestKeyboardFactory _factory;
