#include "serialmatrix.h"
#include "keyboardswitchmessage.h"
#include "keyboardptzmessage.h"
#include "log.h"

static int toPacket(char *buf, KeyboardSwitchMessage &msg)
{
	buf[0] = 0xff;
	buf[1] = 0x00;
	buf[2] = 0x11;
	buf[3] = 0x00;
	buf[4] = (char)msg.camera();
	buf[5] = (char)msg.monitor();
	buf[6] = 0x55;
	buf[7] = buf[1] + buf[2] + buf[3] + buf[4] + buf[5] + buf[6];
	return 0;
}

static int toPacket(char *buf, KeyboardPtzMessage &msg)
{
	buf[0] = 0xff;
	buf[1] = 0x00;
	buf[2] = 0x21;
	buf[3] = 0x00;
	buf[5] = 0x00;
	buf[6] = 0x00;
	// zoom param
	if(msg.zoomInSpeed())
		buf[3] = 0x40;
	else if(msg.zoomOutSpeed())
		buf[3] = 0x80;
	// camera id
	buf[4] = (char)msg.camera();
		// v operation
	if(msg.hSpeed() > 0)
		buf[5] = msg.hSpeed();
	else if(msg.hSpeed() < 0)
		buf[5] = 0x80 - msg.hSpeed();
	// h operation
	if(msg.vSpeed() > 0)
		buf[6] = msg.vSpeed();
	else if(msg.vSpeed() < 0)
		buf[6] = 0x80 - msg.vSpeed();
	buf[7] = buf[1] + buf[2] + buf[3] + buf[4] + buf[5] + buf[6];
	return 0;
}

class MatrixJXJ : public SerialMatrix {
	public:
		MatrixJXJ(Postable *receiver, ModuleInfo *info) : SerialMatrix(receiver, info) {}
		virtual int onReadable(SerialPort& sp)
		{
			char queries = 0;
			static char buf[8] = {0xff, 0x00, 0x11, 0x00, 0x01, 0x01, 0x55, 0x68};
			char reg[] = {0xff, 0x00, 0x41, 0x11, 0x55, 0x00, 0x00, 0xa7};
			static int rcv_cnt = 0;
			Message *msg;
			int len = sp.read(&queries, 1);
			assert(len == 1);
			if(rcv_cnt == 0)
			{
				if((queries & 0xff) != 0x50)
					return 0;
				assert(sp.write(reg, 8) == 8);

				rcv_cnt++;
			}
			else if((0xff & queries) == 0x50)
			{
				if(mFifo.tryget(msg) == 0)
				{
					KeyboardSwitchMessage *smsg = dynamic_cast<KeyboardSwitchMessage*>(msg);
					if(smsg)
					{
						toPacket(buf, *smsg);
					}
					else {
						KeyboardPtzMessage *pmsg = dynamic_cast<KeyboardPtzMessage*>(msg);
						if(pmsg)
						{
							toPacket(buf, *pmsg);
						}
					}
					delete msg;
				}
				sp.write(buf, 8);
			}
			return 0;
		}
		virtual int onExcept(SerialPort& sp)
		{
			ErrLog("serial port has encounter a problem!");
		}
};

/*
class MatrixJXJFactory : public MatrixFactory {
	public:
		virtual Matrix* create(Postable *receiver) {
			if(mInfo)
				return new MatrixJXJ(receiver, mInfo);
			return NULL;
		}
} _factory;
*/
DECL_MATRIX_FACTORY(MatrixJXJ)
