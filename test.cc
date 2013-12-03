#include "gtest/gtest.h"
#include "timevalue.h"
#include "timer.h"
#include "moduleinfo.h"
#include "keyboardmanager.h"
#include "keyboardmessagehelper.h"
#include "matrixmanager.h"
#include "serialkeyboard.h"
#include <algorithm>
#include <queue>
#include <tinyxml.h>
#include "log.h"
using namespace std;

bool LogInitializer::bInitialized = LogInitializer::init();

TEST(ModuleInfo, Test)
{
	ModuleInfo info;
	EXPECT_THROW(info.name(), runtime_error);
	info.open("./Module.xml");
	cout<<info.name()<<endl;
	cout<<info.type()<<endl;
	cout<<info.author()<<endl;
	cout<<info.version()<<endl;
	cout<<info.date()<<endl;
	cout<<info.attribute("baudrate", 4800l) + 1000<<endl;
	cout<<info.attribute("parity", string("E"))<<endl;
	cout<<info.attribute("useThread", false)<<endl;
	string buf;
	info.encode(buf);
	cout<<buf;
}


TEST(MatrixManager, Test)
{	
	int handle;
	MatrixManager mgr("./modules/");
	char path[] = "/dev/ttyUSB0";
	ASSERT_NE((handle = mgr.loadMatrix("matrixcz08", NULL, path)), -1);
	Matrix* mt = mgr.getMatrix();
	ASSERT_NE(mt, (Matrix*)NULL);
	Message *msg = KeyboardMessageHelper::buildSwitchMessage(0, -1, 1, 1);
	ASSERT_NE(msg, (Message*)NULL);
	mt->post(msg);
	sleep(1);
	mgr.unloadMatrix();
}

TEST(KeyboardManager, Test)
{
	Keyboard::Id handle, handle2;
	KeyboardManager mgr("./modules/");
	char path[] = "/dev/ttyUSB0";
	char path1[] = "/dev/ttyS0";
	ASSERT_NE((handle = mgr.loadKeyboard("keyboardhoneywell", NULL, path)), 0);
	ASSERT_NE((handle2 = mgr.loadKeyboard("keyboardjxj", NULL, path1)), 0);
	Keyboard* kb = mgr.getKeyboard(handle);
	ASSERT_NE(kb, (Keyboard*)NULL);
	kb = mgr.getKeyboard(handle2);
	ASSERT_NE(kb, (Keyboard*)NULL);
	ASSERT_EQ(kb->user().priority(), User::LowestPriority);
	mgr.unloadKeyboard(handle);
	mgr.unloadKeyboard(handle2);
}

class TestListener : public SerialPortListener {
	public:
		virtual int onReadable(SerialPort& sp) {
			char buf[32];
			int len = sp.read(buf, sizeof(buf));
			for(int i=0;i<len;++i)
			{
				cout<<buf[i];
				cout.flush();
			}
			cout<<endl;
			return 0;
		}
		virtual int onExcept(SerialPort&) {
			return 0;
		}
};

TEST(SerialPort, Test)
{
	char msg[] = "hello world!";
	TestListener listener;
	SerialPort sp;
	sp.addListener(&listener);
	sp.open("/dev/ttyUSB0", 9600, 8, 1, 'N');
	sleep(1);
	sp.write(msg, sizeof(msg));
	sleep(1);
	sp.close();
}
